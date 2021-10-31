#include <shiken/config.h>
#include <boost/program_options.hpp>
#include <QtPlugin>
#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QHash>
#include <shiken/ui/MainWidget.h>
#include <shiken/ui/StartDialog.h>
#include <shiken/ui/ChooseQuizDialog.h>
#include <shiken/utility/Log.h>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/SettingsDao.h>
#include <shiken/dao/QuizDao.h>
#include <shiken/entities/Quiz.h>
#include <shiken/Shiken.h>

/* Initialize static plugins if it is needed. */
#ifdef SHIKEN_QT_STATIC_PLUGINS
Q_IMPORT_PLUGIN(qsqlite)
#endif

/* Include nwva leak detection code. We don't want to compile it separately. */
#ifdef SHIKEN_USE_NVWA_LEAK_DETECTOR
#  undef new
#  undef ALIGN /* Defined in <arx/Memory.h>. */
#  include <nvwa/debug_new.cpp>
#endif

/* 
 * TODO
 * - create data log for errors in input data format.
 * - doxyfile.
 * - doxygen warnings.
 */

int main(int argc, char** argv) {
  qsrand(qHash(QDateTime::currentDateTime().toString()));

  QApplication app(argc, argv);
  //app.setWindowIcon(QIcon(":/icon.png"));

  SHIKEN_LOG_MESSAGE("Running generator v" << SHIKEN_VERSION << " built on " << __DATE__ << " at " << __TIME__);
  SHIKEN_LOG_MESSAGE("Started on " << QDateTime::currentDateTime().toString());

  /* Parse command line options. */
  bool singleUser = SHIKEN_SINGLE_USER;
  try {
    boost::program_options::options_description desc("Hidden options");
    desc.add_options()
      ("single-user", boost::program_options::value<bool>(&singleUser), "Run in single-user mode.");
    boost::program_options::variables_map vm;
    store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);
  } catch (std::exception &e) {
    SHIKEN_LOG_MESSAGE("Error while parsing command line options: " << e.what());
  }
  SHIKEN_LOG_MESSAGE("Command line parsed, singleUser = " << singleUser);

  /* Initialize data access driver. */
  QScopedPointer<shiken::DataAccessDriver> model(new shiken::DataAccessDriver(SHIKEN_DAT_NAME));
  if(model->settingsDao()->dbVersion() != SHIKEN_DB_VERSION) {
    SHIKEN_LOG_MESSAGE("Found database version " << model->settingsDao()->dbVersion() << ", need version " << SHIKEN_DB_VERSION);
    SHIKEN_LOG_MESSAGE("Clearing database...");

    auto settingsDump = model->settingsDao()->dump();

    model.reset();
    QFile::remove(SHIKEN_DAT_NAME);
    model.reset(new shiken::DataAccessDriver(SHIKEN_DAT_NAME));
    
    model->settingsDao()->import(settingsDump);
  }
  model->settingsDao()->setSingleUser(singleUser);

  /* Prepare context. */
  shiken::Shiken ctx(model.data());

  /* Start wizard. */
  int state = 0;
  while(true) {
    if(state == 0) {
      shiken::StartDialog dlg(&ctx);
      if(dlg.exec() == QDialog::Rejected)
        return 0;
      state++;
    } else if(state == 1) {
      if(model->quizDao()->selectById(model->settingsDao()->quizId()).isNull()) {
        shiken::ChooseQuizDialog dlg(&ctx);
        if(dlg.exec() == QDialog::Rejected)
          return 0;

        state += dlg.backClicked() ? -1 : 1;
      } else {
        state++;
      }
    } else {
      shiken::MainWidget mainWidget(&ctx);
      mainWidget.show();
      return app.exec();
    }
  }

  SHIKEN_LOG_MESSAGE(">>> SHUTTING DOWN <<<");

  return 0;
}
