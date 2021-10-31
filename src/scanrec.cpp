#include "config.h"
#include <iostream>
#include <exception>
#include <QTextStream>
#include "Common.h"
#include "XmlCommons.h"

int main(int argc, char** argv) {
  using namespace boost::program_options;
  using namespace std;

  time_t t;
  srand(static_cast<int>(time(&t)));

  std::string stage;

  QDomDocument document = newDocument();
  QDomElement root = appendElement(document, "scanrec-result");

  try {
    vigra::Size2D maxSize;
    vigra::Rect2D barRect;
    vigra::Rect2D viewRect;
    int maxErrorPercent;
    bool noLma, checkSum;
    string inputFileName, outFileName, keysFileName, viewportFileName;
    int minIterations, maxIterations;

    stage = "Parsing parameters"; 

    options_description desc("Allowed options");
    desc.add_options()
      ("help",                                                              "Produce help message.")
      ("input,i",          value<string>(&inputFileName),                   "Input file name.")
      ("keys,k",           value<string>(&keysFileName),                    "Keypoint file name.")
      ("output,o",         value<string>(&outFileName)->default_value("out.bmp"), 
                                                                            "Output file name.")
      ("size,s",           value<vigra::Size2D>(&maxSize)->default_value(vigra::Size2D(DEFAULT_MAX_SIZE_X, DEFAULT_MAX_SIZE_Y), boost::lexical_cast<string>(DEFAULT_MAX_SIZE_X) + ":" + boost::lexical_cast<string>(DEFAULT_MAX_SIZE_Y)),
                                                                            "Maximal size of an image for keypoint extraction, in format w:h.")
      ("maxerr,m",         value<int>(&maxErrorPercent)->default_value(2),  "Maximal mismatch in reprojected keypoint position relative to image size, in percent.")
      ("nolevmar,l",       bool_switch(&noLma),                             "Don't use Levenberg-Marquardt algorithm for homography optimization.")
      ("position,p",       value<vigra::Rect2D>(&barRect)->default_value(vigra::Rect2D(0, 0, 0, 0), "0:0:0:0"), 
                                                                            "Barcode position in input file, in format x:y:w:h.")
      ("checksum,c",       bool_switch(&checkSum),                          "Check mod 10 checksum.")
      ("min-iterations",   value<int>(&minIterations)->default_value(DEFAULT_MIN_ITERATIONS),
                                                                            "Minimal number of iterations.")
      ("max-iterations",   value<int>(&maxIterations)->default_value(DEFAULT_MAX_ITERATIONS),               
                                                                            "Maximal number of iterations.")
      ("vpfile,f",         value<string>(&viewportFileName),                "Viewport file name.")
      ("viewport,v",       value<vigra::Rect2D>(&viewRect)->default_value(vigra::Rect2D(0, 0, 0, 0), "0:0:0:0"), 
                                                                            "Viewport, in format x:y:w:h.");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if(vm.count("help") > 0 || inputFileName.empty() || keysFileName.empty()) {
      cout << "scanrec - scan recognizer, version " << BRT_VERSION << "." << endl;
      cout << endl;
      cout << "USAGE:" << endl;
      cout << "  scanrec [options]" << endl;
      cout << endl;
      cout << desc << endl;
      return 1;
    }

    /* Load keypoint file. */
    stage = "Loading keypoint file"; 
    acv::Extract<> extract;
    loadExtract(extract, keysFileName);

    fixNegativeSize(&barRect, extract.width(), extract.height());
    fixNegativeSize(&viewRect, extract.width(), extract.height());

    /* Check that barcode lies inside the image. */
    stage = "Checking parameters"; 
    if(!vigra::Rect2D(0, 0, extract.width(), extract.height()).contains(barRect))
      throw logic_error("Specified barcode position lies outside the image boundaries.");

    /* Check that viewport lies inside the image. */
    if(!vigra::Rect2D(0, 0, extract.width(), extract.height()).contains(viewRect))
      throw logic_error("Specified viewport position lies outside the image boundaries.");

    /* Load input image. */
    stage = "Loading input image"; 
    vigra::BRGBImage rgbImage, outImage;
    importImage(rgbImage, inputFileName);
    if(rgbImage.width() == 0 || rgbImage.height() == 0)
      throw logic_error("Could not load input image \"" + inputFileName + "\"");

    /* Match. */
    stage = "Matching"; 
    RansacModel model = match(rgbImage, maxSize, extract, outImage, maxErrorPercent / 100.0f, !noLma);

    /* Save warped image. */
    stage = "Saving warped image"; 
    exportImage(outImage, outFileName);

    /* Recognize barcode. */
    stage = "Recognizing barcode"; 
    try {
      barcode::ItfCode code = recognize(outImage, barRect, minIterations, maxIterations, checkSum);
      
      /* Output. */
      stage = "Writing result"; 
      appendElement(root, "code", QString::fromStdString(code.string()));
    } catch (exception &e) {
      /* Recognition failed. */
      appendElement(root, "error", "1");
      appendElement(root, "error-string", e.what());
    }

    /* Output. */
    appendElement(root, "width", QString::number(extract.width()));
    appendElement(root, "height", QString::number(extract.height()));
    /* Model defines a rotation transformation, so here we have sqr(SCALE * sin(ALPHA)) + sqr(SCALE * cos(ALPHA)) = sqr(SCALE). */
    appendElement(root, "scale", QString::number(sqrt(arx::sqr(model(0, 0)) + arx::sqr(model(0, 1)))));

    /* Save viewport image if needed. */
    stage = "Generating & saving viewport image"; 
    if(!viewportFileName.empty()) {
      vigra::BRGBImage vpImage(viewRect.size());
      copyImage(srcImageRange(outImage, viewRect), destImage(vpImage));
      exportImage(vpImage, viewportFileName);
    }
  } catch (exception& e) {
    appendElement(root, "error", "2");
    appendElement(root, "error-string", QString::fromStdString("error on stage \"" + stage + "\": " + e.what()));
  } catch(...) {
    appendElement(root, "error", "2");
    appendElement(root, "error-string", QString::fromStdString("error on stage \"" + stage + "\": Unknown error"));
  }

  QTextStream qStdOut(stdout);
  document.save(qStdOut, -1);

  return 0;
}
