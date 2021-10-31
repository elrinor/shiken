#ifndef SHIKEN_WORKER_H
#define SHIKEN_WORKER_H

#include <shiken/config.h>
#include <QObject>
#include <arx/ScopeExit.h>

class QNetworkReply;

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// WorkerBase
// -------------------------------------------------------------------------- //
  /**
   * Qt does not support signals and slots in template classes, that's why
   * WorkerBase class was introduced. It contains definitions of signals and 
   * slots used in workers and some useful common functionality. 
   */
  class WorkerBase: public QObject {
    Q_OBJECT;
  public:
    WorkerBase(Shiken *ctx, int maximum):
      mCtx(ctx), mMaximum(maximum), mSucceeded(true), mCanceled(false) {}

    bool succeeded() {
      return mSucceeded;
    }

    bool canceled() {
      return mCanceled;
    }

    int maximum() const {
      return mMaximum;
    }

  protected:
    void criticalConnectionError(QNetworkReply *reply);
    
    void criticalInvalidReplyFormat();

    Shiken *ctx() const {
      return mCtx;
    }

  Q_SIGNALS:
    void started(int maximum);
    void advanced(int amount);
    void finished();
    void critical(QString message, QString details = QString());

  private:
    friend class WorkerRunnerBase;

    void setSucceeded(bool succeeded) {
      mSucceeded = succeeded;
    }

    void setCanceled(bool canceled) {
      mCanceled = canceled;
    }

    Shiken *mCtx;
    int mMaximum;
    bool mSucceeded;
    volatile bool mCanceled;
  };

  template<class ResultType>
  class WorkerRunner;

// -------------------------------------------------------------------------- //
// Worker
// -------------------------------------------------------------------------- //
  /**
   * Worker is a base class for all shiken workers.
   *
   * @tparam ResultType                Result type for this worker.
   */
  template<class ResultType>
  class Worker: public WorkerBase {
  public:
    /**
     * Constructor.
     * 
     * If the given total number of steps is non-negative, then started(int) 
     * and finished() signals will be emitted automatically. If it's negative, 
     * then they will have to be emitted manually from the derived class.
     *
     * @param ctx                      Application context.
     * @param maximum                  Total number of steps a worker will 
     *                                 perform. 
     */
    Worker(Shiken *ctx, int maximum): 
      WorkerBase(ctx, maximum) {}

    typedef ResultType result_type;

    virtual result_type operator()() = 0;

  private:
    friend class WorkerRunner<result_type>;

    result_type run() {
      if(maximum() >= 0)
        Q_EMIT started(maximum());

      ARX_SCOPE_EXIT(&) {
        if(this->maximum() >= 0)
          Q_EMIT this->finished();
      };

      return operator()();
    }
  };

} // namespace shiken

#endif // SHIKEN_WORKER_H
