#ifndef SHIKEN_CONFIG_H
#define SHIKEN_CONFIG_H

#include "../config.h"

// -------------------------------------------------------------------------- //
// Configuration
// -------------------------------------------------------------------------- //
/**
 * Don't perform logging?
 */
//#define SHIKEN_NO_LOGGING

/**
 * Use NVWA leak detector?
 */
#ifdef _DEBUG
//#  define SHIKEN_USE_NVWA_LEAK_DETECTOR
#endif

/**
 * Single-user by default?
 */
#ifndef SHIKEN_SINGLE_USER
#  define SHIKEN_SINGLE_USER 0
#endif


// -------------------------------------------------------------------------- //
// Shortcuts
// -------------------------------------------------------------------------- //
/**
 * Shortcut for QString construction from wchar_t string literals.
 */
#define QS(STRING) QString::fromWCharArray(L ## STRING)

/**
 * Shortcut for QString construction from UTF8-encoded char string literals.
 */
#define QS8(STRING) QString::fromUtf8(STRING)


// -------------------------------------------------------------------------- //
// Globals
// -------------------------------------------------------------------------- //
/**
 * Shiken version string.
 */
#define SHIKEN_VERSION "0.4.23"

/**
 * Shiken xsd namespace
 */
#define SHIKEN_XSD_NAMESPACE "http://www.elric.ru/xsd/shiken"

/**
 * Shiken version as an integer.
 */
#define SHIKEN_INT_VERSION 423

/**
 * Version of shiken database file required for this shiken version to work.
 */
#define SHIKEN_DB_VERSION "0.4.23"

/* Names of web service actions used by shiken. */
#define SHIKEN_UPLOAD_COMMAND QS("UploadScan")
#define SHIKEN_QUIZZES_COMMAND QS("GetCurrentQuizzesXml")
#define SHIKEN_SCANS_COMMAND QS("GetScansXml")
#define SHIKEN_USERS_COMMAND QS("GetStudentsXml")
#define SHIKEN_VERSION_COMMAND QS("GetLatestClientVersionXml")

/**
 * Name of main SQL connection.
 */
#define SHIKEN_SQL_CONNECTION_NAME "shiken_default_connection"

/**
 * Name of shiken SQLite database file.
 */
#define SHIKEN_DAT_NAME "generator.dat"

/**
 * Name of log file for shiken.
 */
#define SHIKEN_LOG_NAME "generator.log"

/**
 * Date/time format used in barcodes on answer sheets.
 */
#define SHIKEN_BARCODE_DATE_TIME_FORMAT "yyMMddhhmmsszzz"
#define SHIKEN_BARCODE_DATE_TIME_FORMAT_LENGTH (sizeof(SHIKEN_BARCODE_DATE_TIME_FORMAT) - 1)

/**
 * Date/time format used for (de)serialization.
 */
#define SHIKEN_DATE_TIME_FORMAT "yyyyMMddhhmmsszzz"

/**
 * Barcode ID for answer sheet barcodes. That's the first two digits of the
 * barcode.
 */
#define SHIKEN_TEST_BARCODE_ID 10

/**
 * Default login for a user that was not registered for current test, but
 * somehow ended up in the database (due to user error, most probably).
 */
#define SHIKEN_UNKNOWN_LOGIN QS8("\xd0\x9d\xd0\xb5\xd0\xb8\xd0\xb7\xd0\xb2\xd0\xb5\xd1\x81\xd1\x82\xd0\xbd\xd1\x8b\xd0\xb9\x20\xd0\xbb\xd0\xbe\xd0\xb3\xd0\xb8\xd0\xbd")

/* Keys for all settings. */
#define SHIKEN_LOGIN_KEY                     "login"
#define SHIKEN_PROXIES_KEY                   "proxies"
#define SHIKEN_USER_PROXY_ADDRESS_KEY        "user_proxy_address"
#define SHIKEN_USER_PROXY_PORT_KEY           "user_proxy_port"
#define SHIKEN_USER_PROXY_TYPE_KEY           "user_proxy_type"
#define SHIKEN_PROXY_LOGIN_KEY               "proxy_login"
#define SHIKEN_PROXY_PASSWORD_KEY            "proxy_password"
#define SHIKEN_DB_VERSION_KEY                "version"
#define SHIKEN_PAGE_COUNT_KEY                "page_count"
#define SHIKEN_HELP_URL_KEY                  "help_url"
#define SHIKEN_TARGET_URL_KEY                "target_url"
#define SHIKEN_BINARY_URL_KEY                "binary_url"
#define SHIKEN_MAX_KEY_IMAGE_WIDTH_KEY       "max_key_image_width"
#define SHIKEN_MAX_KEY_IMAGE_HEIGHT_KEY      "max_key_image_height"
#define SHIKEN_MAX_RANSAC_ERROR_KEY          "max_ransac_error"
#define SHIKEN_SCANS_UPDATE_INTERVAL_MSECS_KEY "scans_update_interval_msecs"


// -------------------------------------------------------------------------- //
// Defaults
// -------------------------------------------------------------------------- //
/**
 * Default number of answer sheets to print per student. Can be changed in 
 * database file.
 */
#define SHIKEN_DEFAULT_PAGE_COUNT 3

/**
 * Default help url. Can be changed in database file.
 */
#define SHIKEN_DEFAULT_HELP_URL "http://test.mioo.ru/download/generator.pdf"

/**
 * Default web service url. Can be changed in database file.
 */
#define SHIKEN_DEFAULT_TARGET_URL "http://test.mioo.ru/or/mioo/"

/**
 * Default url to a new version. Can be changed in database file.
 */
#define SHIKEN_DEFAULT_BINARY_URL "http://test.mioo.ru/download/generator.exe"

/**
 * Default max width for image used for scan transformation estimation.
 */
#define SHIKEN_DEFAULT_MAX_KEY_IMAGE_WIDTH 1024

/**
 * Default max height for image used for scan transformation estimation.
 */
#define SHIKEN_DEFAULT_MAX_KEY_IMAGE_HEIGHT 1024

/**
 * Default RANSAC max error used for match filtering in scan transformation
 * estimation. Reasonable values are in range [0.01, 0.05].
 */
#define SHIKEN_DEFAULT_MAX_RANSAC_ERROR 0.02

/**
 * Default interval in milliseconds between consecutive requests for a list 
 * of scans that were uploaded to the server.
 */
#define SHIKEN_DEFAULT_SCANS_UPDATE_INTERVAL_MSECS (5 * 60 * 1000)


// -------------------------------------------------------------------------- //
// Language features & library configuration
// -------------------------------------------------------------------------- //
/* Use C++0x features without changing the syntax! */
#ifdef _MSC_VER
#  undef NULL
#  define NULL nullptr
#endif

/* Prevent Qt from defining its own foreach & provide meaningful error
 * messages when we forget to include <arx/Foreach.h>. */
#define foreach(a, b) ARX_FOREACH_H_NOT_INCLUDED;

/* Use expression-template-based string concatenation in Qt. */
#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

/* Define override specifier for MSVC. */
#ifdef _MSC_VER
#  define OVERRIDE override
#else
#  define OVERRIDE
#endif

/* Use leak detection from nvwa. */
#ifdef SHIKEN_USE_NVWA_LEAK_DETECTOR
#  ifdef _MSC_VER
#    /* The following header defines operator new, therefore it must be 
#     * included before new is redefined to something evil by nvwa. */
#    include <crtdbg.h> 
#  endif
#  /* The following headers either define operator new or manipulate it in some
#   * way, so they also must be included before nvwa. */
#  include <boost/type_traits/has_new_operator.hpp>
#  include <Eigen/Dense>
#  include <arx/Memory.h>
#  define _DEBUG_NEW_TAILCHECK 16
#  include <nvwa/debug_new.h>
#endif // SHIKEN_USE_NVWA_LEAK_DETECTOR

#endif // SHIKEN_CONFIG_H
