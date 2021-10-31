#include "config.h"
#include <cstdlib> /* for srand() */
#include <ctime>   /* for time() */
#include <iostream>
#include <exception>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <vigra/stdimage.hxx>
#include "acv/Extractor.h"
#include "Common.h"

int main(int argc, char** argv) {
  using namespace boost::program_options;
  using namespace std;

  time_t t;
  srand(static_cast<int>(time(&t)));

  try {
    vigra::Size2D maxSize;

    options_description desc("Allowed options");
    desc.add_options()
      ("help",                     "produce help message")
      ("input,i", value<string>(), "input file name")
      ("size,s",  value<vigra::Size2D>(&maxSize)->default_value(vigra::Size2D(DEFAULT_MAX_SIZE_X, DEFAULT_MAX_SIZE_Y), boost::lexical_cast<string>(DEFAULT_MAX_SIZE_X) + ":" + boost::lexical_cast<string>(DEFAULT_MAX_SIZE_Y)), 
                                   "maximal size of an image for keypoint extraction, in format w:h")
      ("draw,d",  value<string>(), "draw keypoints and save result into a file with the given name");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if(vm.count("help") > 0 || vm.count("input") < 1) {
      cout << "kpextract - keypoint extractor, version " << BRT_VERSION << "." << endl;
      cout << endl;
      cout << "USAGE:" << endl;
      cout << "  kpextract [options]" << endl;
      cout << endl;
      cout << desc << endl;
      return 1;
    }

    /* Load input image. */
    vigra::BRGBImage image;
    importImage(image, vm["input"].as<string>());

    /* Extract keypoints from input image. */
    acv::Extract<> extract;
    extractKeypoints(image, maxSize, extract);

    /* Draw keypoints if needed. */
    if(vm.count("draw") > 0) {
      importImage(image, vm["input"].as<string>());
      markKeypoints(extract.keypoints(), image, vigra::RGBValue<vigra::UInt8>(0, 0, 255));
      exportImage(image, vm["draw"].as<string>());
    }

    cout << extract;
  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch(...) {
    cerr << "error: Exception of unknown type." << endl;
    return 1;
  }

  return 0;
}