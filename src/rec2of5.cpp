#include "config.h"
#include <cstdlib> /* for srand() */
#include <ctime>   /* for time() */
#include <iostream>
#include <exception>
#include <boost/program_options.hpp>
#include <arx/ext/Vigra.h>
#include <arx/ext/Qt.h>
#include "Common.h"

int main(int argc, char** argv) {
  using namespace boost::program_options;
  using namespace std;

  time_t t;
  srand(static_cast<int>(time(&t)));

  try {
    std::string inputFileName;
    vigra::Rect2D barRect;
    bool checkSum;
    int minIterations, maxIterations;

    options_description desc("Allowed options");
    desc.add_options()
      ("help",                                               "Produce help message.")
      ("input,i",          value<string>(&inputFileName),    "Input file name.")
      ("position,p",       value<vigra::Rect2D>(&barRect)->default_value(vigra::Rect2D(0, 0, 0, 0), "0:0:0:0"), 
                                                             "Barcode position in input file, in format x:y:w:h.")
      ("checksum,c",       bool_switch(&checkSum),           "Check mod 10 checksum.")
      ("min-iterations",   value<int>(&minIterations)->default_value(DEFAULT_MIN_ITERATIONS),  
                                                             "Minimal number of iterations.")
      ("max-iterations",   value<int>(&maxIterations)->default_value(DEFAULT_MAX_ITERATIONS), 
                                                             "Maximal number of iterations.");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if(vm.count("help") > 0 || inputFileName.empty()) {
      cout << "rec2of5 - Interleaved 2 of 5 barcode recognizer, version " << BRT_VERSION << "." << endl;
      cout << endl;
      cout << "USAGE:" << endl;
      cout << "  rec2of5 [options]" << endl;
      cout << endl;
      cout << desc << endl;
      return 1;
    }

    vigra::BImage image;
    importImage(image, inputFileName);

    fixNegativeSize(&barRect, image.size());
    if(!vigra::Rect2D(vigra::Point2D(0, 0), image.size()).contains(barRect))
      throw logic_error("Specified barcode position lies outside the input image boundaries.");

    barcode::ItfCode code = recognize(image, barRect, minIterations, maxIterations, checkSum);
    cout << code.string();
  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch(...) {
    cerr << "error: Exception of unknown type." << endl;
    return 1;
  }

  return 0;
}