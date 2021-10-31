#include "config.h"
#include <iostream>
#include <exception>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include "Common.h"

int main(int argc, char** argv) {
  using namespace boost::program_options;
  using namespace std;

  try {
    vigra::Size2D maxSize;
    int maxErrorPercent;
    bool noLma;

    options_description desc("Allowed options");
    desc.add_options()
      ("help",                                                       "produce help message")
      ("input,i",    value<string>(),                                "input file name")
      ("keys,k",     value<string>(),                                "keypoint file name")
      ("output,o",   value<string>()->default_value("out.bmp"),      "output file name")
      ("size,s",     value<vigra::Size2D>(&maxSize)->default_value(vigra::Size2D(DEFAULT_MAX_SIZE_X, DEFAULT_MAX_SIZE_Y), boost::lexical_cast<string>(DEFAULT_MAX_SIZE_X) + "+" + boost::lexical_cast<string>(DEFAULT_MAX_SIZE_Y)), 
                                                                     "maximal size of an image for keypoint extraction, in format w:h")
      ("maxerr,m",   value<int>(&maxErrorPercent)->default_value(2), "maximal mismatch in reprojected keypoint position relative to image size, in percent")
      ("nolevmar,l", bool_switch(&noLma),                            "don't use Levenberg-Marquardt algorithm for homography optimization");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if(vm.count("help") > 0 || vm.count("input") < 1 || vm.count("keys") < 1) {
      cout << "kpmatch - keypoint matcher, version " << BRT_VERSION << "." << endl;
      cout << endl;
      cout << "USAGE:" << endl;
      cout << "  kpmatch [options]" << endl;
      cout << endl;
      cout << desc << endl;
      return 1;
    }

    /* Load keypoint file. */
    acv::Extract<> extract;
    loadExtract(extract, vm["keys"].as<string>());

    /* Load input image. */
    vigra::BRGBImage srcImage, outImage;
    importImage(srcImage, vm["input"].as<string>());

    /* Match. */
    match(srcImage, maxSize, extract, outImage, maxErrorPercent / 100.0f, !noLma);

    /* Output. */
    exportImage(outImage, vm["output"].as<string>());
  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch(...) {
    cerr << "error: Exception of unknown type." << endl;
    return 1;
  }

  return 0;
}