#include "config.h"
#include <iostream>
#include <exception>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <vigra/stdimage.hxx>
#include <arx/Foreach.h>
#include "barcode/ItfCode.h"
#include "ImageUtils.h"
#include "Common.h"

/**
 * Validation function for barcode::ItfCode command line parameter.
 */
void validate(boost::any& v, const std::vector<std::string>& values, barcode::ItfCode* target_type, int) {
  using namespace boost::program_options;

  /* Check that it's the only one code. */
  validators::check_first_occurrence(v);
  const std::string& s = validators::get_single_string(values);

  /* Check for digits. */
  std::locale defaultLocale;
  foreach(char c, s)
    if(!isdigit(c, defaultLocale))
      throw invalid_option_value("Non-digit character in code.");

  /* Ok. */
  v = barcode::ItfCode(s);
}

namespace barcode {
  /* ADL chokes for some reason if we don't explicitly introduce it here... */
  using ::validate; 
}

int main(int argc, char** argv) {
  using namespace boost::program_options;
  using namespace std;

  try {
    options_description desc("Allowed options");
    desc.add_options()
      ("help",                                                  "produce help message")
      ("code,i",     value<barcode::ItfCode>(),                 "digit code (required)")
      ("output,o",   value<string>()->default_value("out.bmp"), "output file name")
      ("checksum,c", bool_switch(),                             "append mod 10 checksum")
      ("width,w",    value<int>()->default_value(1),            "width of a thin bar, in pixels")
      ("height,h",   value<int>()->default_value(50),           "height of barcode area, in pixels")
      ("space,s",    value<int>()->default_value(5),            "spacing between barcode and label, in pixels")
      ("lheight,l",  value<int>()->default_value(15),           "height of label, in pixels");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if(vm.count("help") > 0 || vm.count("code") == 0) {
      cout << "gen2of5 - Interleaved 2 of 5 barcode image generator, version " << BRT_VERSION << "." << endl;
      cout << endl;
      cout << "USAGE:" << endl;
      cout << "  gen2of5 [options]" << endl;
      cout << endl;
      cout << desc << endl;
      return 1;
    }

    barcode::ItfCode bCode = vm["code"].as<barcode::ItfCode>();
    if(vm["checksum"].as<bool>()) {
      if(bCode.size() % 2 == 0)
        bCode.addDigit(0);
      bCode.addDigit(bCode.mod10CheckSum());
    } else {
      if(bCode.size() % 2 == 1)
        bCode.addDigit(0);
    }

    vigra::BImage img;
    bCode.createImage(img, vm["width"].as<int>(), vm["height"].as<int>(), vm["space"].as<int>(), vm["lheight"].as<int>());
    exportImage(img, vm["output"].as<string>());
  } catch(exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch(...) {
    cerr << "error: Exception of unknown type." << endl;
    return 1;
  }

  return 0;
}