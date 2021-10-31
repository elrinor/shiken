#ifndef SHIKEN_WORKER_RUNNER_H
#define SHIKEN_WORKER_RUNNER_H

#include <shiken/config.h>
#include <cassert>
#include <functional> /* For std::bind. */
#include <QScopedPointer>
#include <QFuture>
#include <QtConcurrentRun>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <arx/ScopeExit.h>
#include "Worker.h"

class QProgressDialog;
class QEventLoop;
class QPushButton;
class QWidget;

namespace shiken {
// -------------------------------------------------------------------------- //
// WorkerRunnerBase
// -------------------------------------------------------------------------- //
  /**
   * Qt does not support signals and slots in template classes, that's why
   * this class was introduced.
   */
  class WorkerRunnerBase: public QObject {
    Q_OBJECT;
  public:
    WorkerRunnerBase(WorkerBase *worker, QWidget *widget, QString progressMessage, QString errorMessage, bool cancellable);

    ~WorkerRunnerBase();

  protected:
    /**
     * Starts a local event loop.
     */
    void exec() const;

    template<class ResultType>
    Worker<ResultType> *worker() const {
      assert(dynamic_cast<Worker<ResultType> *>(mWorker) != NULL);

      return static_cast<Worker<ResultType> *>(mWorker);
    }

  protected Q_SLOTS:
    void onStarted(int maximum) const;

    void onAdvanced(int amount) const;

    void onCritical(QString message, QString details) const;

    void onCanceled() const;

  private:
    WorkerBase *mWorker;
    QWidget *mWidget;
    QString mErrorMessage;

    QScopedPointer<QProgressDialog> mDialog;
    QPushButton *mCancelButton;
    QScopedPointer<QEventLoop> mLoop;
  };


// -------------------------------------------------------------------------- //
// WorkerRunner
// -------------------------------------------------------------------------- //
  /**
   * WorkerRunner class executes the given Worker in a separate thread and
   * provides such functionality as a progress bar, error reporting and
   * cancellation handling.
   */
  template<class ResultType>
  class WorkerRunner: public WorkerRunnerBase {
  public:
    WorkerRunner(Worker<ResultType> *worker, QWidget *widget, QString progressMessage, QString errorMessage, bool cancellable): 
      WorkerRunnerBase(worker, widget, progressMessage, errorMessage, cancellable) {}

    ResultType operator()() {
      Worker<ResultType> *localWorker = worker<ResultType>();

      /** A small note on where all this mess came from. 
       * 
       * We want to use thread pool for running workers because creating a new
       * thread every time a worker is created is a waste of resources. 
       * Besides, we will also have to create thread-specific data anew 
       * (an example would be a QNetworkAccessManager created for each
       * thread by ConnectionManager).
       * 
       * So, the option is to use QThreadPool. But it doesn't let us access
       * QThreads directly. We can only feed it a QRunnable instance and pray.
       * This also means that we'll have to roll out some hand-written 
       * synchronization, write a class that would wrap a worker into 
       * QRunnable, etc... That's a mess.
       * 
       * There is another option - to use QtConcurrent::run. However, for
       * signals to work properly we need to move worker to the thread it's 
       * being executed in. We have no means to know which thread a worker is
       * going to end up in from outside the worker's code, so we still have
       * to resort to hand-written synchronization mess. Still, it's the least
       * messy solution. */
      
      /* Mutex & wait condition for synchronization of poolThread 
       * initialization. */
      QMutex poolMutex;
      QWaitCondition waitCondition;

      /* Mutex for synchronization of moveToThread operation. */
      QMutex moveMutex;

      /* QThread from the pool that the worker will be run in. */
      QThread *poolThread = NULL;

      /* The QThread that the worker is assigned to. */
      QThread *const workerThread = localWorker->thread();

      poolMutex.lock();
      moveMutex.lock();

      /* C++0x lambda is wrapped in std::bind in the following invocation 
       * because QtConcurrent::run requires nested result_type to be defined 
       * for the functor being invoked. Lambdas do not supply it, result 
       * of std::bind does. */
      QFuture<ResultType> future = QtConcurrent::run(
        std::bind<ResultType>(
          [&]() -> ResultType {
            /* Initialize poolThread with a pointer to this thread. */
            poolThread = QThread::currentThread();

            /* Wait for the main thread to start waiting on wait condition. */
            poolMutex.lock();
            poolMutex.unlock();

            /* Wake main thread. */
            waitCondition.wakeAll();
            
            /* Wait until main thread moves worker to this thread. */
            moveMutex.lock();
            moveMutex.unlock();
            assert(localWorker->thread() == QThread::currentThread());

            /* ResultType can be void, therefore result of worker invocation
             * cannot be stored in a temporary without trickery.
             *
             * We compensate for that by using scope exit to restore worker's
             * original thread after worker invocation. */
            ARX_SCOPE_EXIT(=) {
              localWorker->moveToThread(workerThread);
              assert(localWorker->thread() == workerThread);
            };

            return localWorker->run();
          }
        )
      );

      /* Wait until poolThread is initialized. */
      waitCondition.wait(&poolMutex);
      poolMutex.unlock();
      assert(poolThread != NULL);

      /* We need this trickery for signal-slot mechanism to work properly.
       * Otherwise worker and receiver objects will end up in the same 
       * thread, thus effectively turning all signal emissions into direct
       * function calls. */
      localWorker->moveToThread(poolThread);
      assert(localWorker->thread() == poolThread);
      
      /* Wake the pool thread. */
      moveMutex.unlock();

      /* Wait for the worker to finish. */
      exec();
      future.waitForFinished();
      assert(localWorker->thread() == workerThread);

      return result(future);
    }

  protected:
    template<class OtherResultType>
    OtherResultType result(const QFuture<OtherResultType> &future) {
      return future.result();
    }

    void result(const QFuture<void> &) {
      return;
    }
  };

  /**
   * Executes the given Worker with the given parameters.
   *
   * @param worker                     Worker to execute.
   * @param progressMessage            Message for the progress bar. If empty, 
   *                                   no progress bar will be shown.
   * @param errorMessage               Message template for error reporting,
   *                                   must contain a %1 place marker that
   *                                   will be replaced with the actual error
   *                                   message. If empty, errors won't be shown.
   * @param cancellable                Is the process cancellable? If true,
   *                                   then there will be a "cancel" button on 
   *                                   the progress dialog.
   */
  template<class ResultType>
  ResultType runWorker(Worker<ResultType> &worker, QWidget *widget = NULL, QString progressMessage = QString(), QString errorMessage = QString(), bool cancellable = false) {
    return WorkerRunner<ResultType>(&worker, widget, progressMessage, errorMessage, cancellable)();
  }


} // namespace shiken

#endif // SHIKEN_WORKER_RUNNER_H
