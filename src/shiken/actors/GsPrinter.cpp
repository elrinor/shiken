#include "GsPrinter.h"
#include <cassert>
#include <Windows.h>
#include <iapi.h>
#include <ierrors.h>
#include <QFile>
#include <arx/ScopeExit.h>

#define GS_DEFAULT_DLL_NAME "gsdll32.dll"

namespace {
  /**
   * Structure containing function pointers into Ghostscript dll.
   */
  struct GSDLL {
    HINSTANCE hModule;	/* DLL module handle */
    PFN_gsapi_revision revision;
    PFN_gsapi_new_instance new_instance;
    PFN_gsapi_delete_instance delete_instance;
    PFN_gsapi_set_stdio set_stdio;
    PFN_gsapi_set_poll set_poll;
    PFN_gsapi_set_display_callback set_display_callback;
    PFN_gsapi_init_with_args init_with_args;
    PFN_gsapi_run_string run_string;
    PFN_gsapi_exit exit;
    PFN_gsapi_set_visual_tracer set_visual_tracer;
  };

  /**
   * Unloads Ghostscript dll and clears the gsdll structure.
   */
  void unloadDll(GSDLL *gsdll) {
    /* Set functions to NULL to prevent usage. */
    gsdll->revision = NULL;
    gsdll->new_instance = NULL;
    gsdll->delete_instance = NULL;
    gsdll->init_with_args = NULL;
    gsdll->run_string = NULL;
    gsdll->exit = NULL;
    gsdll->set_stdio = NULL;
    gsdll->set_poll = NULL;
    gsdll->set_display_callback = NULL;
    gsdll->set_visual_tracer = NULL;

    if(gsdll->hModule != NULL)
      FreeLibrary(gsdll->hModule);
    gsdll->hModule = NULL;
  }

  /**
   * Loads Ghostscript dll and fills gsdll structure.
   *
   * @param lastError[out]             Last error occured during dll loading,
   *                                   if any.
   * @returns                          Zero if succeeded, non-zero otherwise.
   */
  int loadDll(GSDLL *gsdll, QString *lastError) {
    assert(lastError != NULL);

    /* Don't load if already loaded. */
    if(gsdll->hModule)
      return 0;

    /* First try to load DLL from the same directory as EXE. */
    WCHAR buffer[1024];
    int bufferSize = sizeof(buffer) / sizeof(buffer[0]);
    if(GetModuleFileNameW(GetModuleHandleW(NULL), buffer, bufferSize) != 0) {
      buffer[bufferSize - 1] = 0;

      QString dllName = QString::fromWCharArray(buffer);
      int pos = dllName.lastIndexOf('\\');
      if(pos != -1) {
        dllName.chop(dllName.size() - pos - 1);
        dllName += GS_DEFAULT_DLL_NAME;
      } else {
        dllName = GS_DEFAULT_DLL_NAME;
      }

      gsdll->hModule = LoadLibraryW(reinterpret_cast<LPCWSTR>(dllName.utf16()));
    }
  
    /* Next try to load DLL with name in registry or environment variable. */
    if(gsdll->hModule < (HINSTANCE) HINSTANCE_ERROR) {
      QString dllName = QString::fromLocal8Bit(qgetenv("GS_DLL"));
      if(!dllName.isEmpty())
        gsdll->hModule = LoadLibraryW(reinterpret_cast<LPCWSTR>(dllName.utf16()));
    }

    /* Finally try the system search path. */
    if(gsdll->hModule < (HINSTANCE) HINSTANCE_ERROR)
      gsdll->hModule = LoadLibraryA(GS_DEFAULT_DLL_NAME);

    /* Failed. */
    if(gsdll->hModule < (HINSTANCE) HINSTANCE_ERROR) {
      DWORD err = GetLastError();
      *lastError = QString("Can't load Ghostscript DLL, LoadLibrary error code %1.").arg(err);
      gsdll->hModule = NULL;
      return 1;
    }

    /* DLL is now loaded */

    /* Get pointers to functions. */
    gsdll->revision = (PFN_gsapi_revision) GetProcAddress(gsdll->hModule, "gsapi_revision");
    if(gsdll->revision == NULL) {
      *lastError = "Can't find gsapi_revision.";
      unloadDll(gsdll);
      return 1;
    }

    /* Check DLL version. */
    gsapi_revision_t rv;
    if(gsdll->revision(&rv, sizeof(rv)) != 0) {
      *lastError = "Unable to identify Ghostscript DLL revision.";
      unloadDll(gsdll);
      return 1;
    }

    /*
    if (rv.revision != GSREVISION) {
      // No checking.
    }
    */

    /* Load other functions */
    gsdll->new_instance = (PFN_gsapi_new_instance) GetProcAddress(gsdll->hModule, "gsapi_new_instance");
    if(gsdll->new_instance == NULL) {
      *lastError = "Can't find gsapi_new_instance.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->delete_instance = (PFN_gsapi_delete_instance) GetProcAddress(gsdll->hModule, "gsapi_delete_instance");
    if(gsdll->delete_instance == NULL) {
      *lastError = "Can't find gsapi_delete_instance.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->set_stdio = (PFN_gsapi_set_stdio) GetProcAddress(gsdll->hModule, "gsapi_set_stdio");
    if(gsdll->set_stdio == NULL) {
      *lastError = "Can't find gsapi_set_stdio.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->set_poll = (PFN_gsapi_set_poll) GetProcAddress(gsdll->hModule, "gsapi_set_poll");
    if(gsdll->set_poll == NULL) {
      *lastError = "Can't find gsapi_set_poll.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->set_display_callback = (PFN_gsapi_set_display_callback) GetProcAddress(gsdll->hModule, "gsapi_set_display_callback");
    if(gsdll->set_display_callback == NULL) {
      *lastError = "Can't find gsapi_set_display_callback.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->init_with_args = (PFN_gsapi_init_with_args) GetProcAddress(gsdll->hModule, "gsapi_init_with_args");
    if(gsdll->init_with_args == NULL) {
      *lastError = "Can't find gsapi_init_with_args.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->run_string = (PFN_gsapi_run_string) GetProcAddress(gsdll->hModule, "gsapi_run_string");
    if(gsdll->run_string == NULL) {
      *lastError = "Can't find gsapi_run_string.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->exit = (PFN_gsapi_exit) GetProcAddress(gsdll->hModule, "gsapi_exit");
    if(gsdll->exit == NULL) {
      *lastError = "Can't find gsapi_exit.";
      unloadDll(gsdll);
      return 1;
    }

    gsdll->set_visual_tracer = (PFN_gsapi_set_visual_tracer) GetProcAddress(gsdll->hModule, "gsapi_set_visual_tracer");
    if(gsdll->set_visual_tracer == NULL) {
      *lastError = "Can't find gsapi_set_visual_tracer.";
      unloadDll(gsdll);
      return 1;
    }

    return 0;
  }

} // namespace `anonymous-namespace`

namespace shiken {

  void GsPrinter::operator() () {
    /* Spawn ghostscript dll. */
    if(!QFile::exists(GS_DEFAULT_DLL_NAME))
      QFile::copy(":/gsdll32.dll", GS_DEFAULT_DLL_NAME);

    /* Load dll. */
    GSDLL gsdll;
    memset(&gsdll, 0, sizeof(gsdll));
    QString lastError;
    if(loadDll(&gsdll, &lastError) != 0) {
      Q_EMIT critical(lastError);
      return;
    }

    /* Create Ghostscript instance. */
    void *instance;
    if(gsdll.new_instance(&instance, NULL) < 0) {
      Q_EMIT critical("Can't create Ghostscript instance.");
      return;
    }

    /* Ghostscript uses 8-bit encoding.
     * We need to check that we can actually pass fileName to it. */
    QString fileName = mFileName;
    if(QString::fromLocal8Bit(fileName.toLocal8Bit()) != fileName) {
      QString fileName = "C:\\gs" + QString::number(qrand()) + QString::number(qrand()) + QString::number(qrand()) + ".pdf";
      QFile::copy(mFileName, fileName); 

      /* We don't check for success here as it's unclear what to do if copy fails. */
    } 

    /* Prepare arguments. */
    QString outputFile = "-sOutputFile=\\\\spool\\" + mPrinterName;
    QByteArray outputFileLocal = outputFile.toLocal8Bit();
    QByteArray inputFileLocal = fileName.toLocal8Bit();

    char* gsargv[] = {
      "", /* actual value doesn't matter */
      "-sDEVICE=mswinpr2",
      "-dSAFER",
      "-dNOPAUSE",
      "-dBATCH",
      outputFileLocal.data(),
      inputFileLocal.data(),
      NULL
    };

    /* Run Ghostscript. */
    int code = gsdll.init_with_args(instance, 7, gsargv);
    int exitCode = gsdll.exit(instance);

    /* Handle errors. */
    if(code == 0 || (code == e_Quit && exitCode != 0))
      code = exitCode;
    switch(code) {
    case 0:
    case e_Info:
    case e_Quit:
      break; /* No error. */
    case e_Fatal:
      Q_EMIT critical("Ghostscript fatal error.");
      break;
    default:
      Q_EMIT critical("Ghostscript unknown error.");
      break;
    }

    /* Clean up. */
    gsdll.delete_instance(instance);
    unloadDll(&gsdll);
    if(mFileName != fileName)
      QFile::remove(fileName);

    Q_EMIT advanced(1);
  }

} // namespace shiken
