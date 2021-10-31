#define EIGEN_DONT_ALIGN
#include "config.h"
#include "shiken/config.h"
#include <cstdlib> /* for srand() */
#include <ctime>   /* for time() */
#include <iostream>
#include <exception>
#include <boost/program_options.hpp>
#include <vigra/labelimage.hxx>
#include <arx/Memory.h>
#include "shiken/RegionInfo.h"
#include "Common.h"

#include "acv/HomographyLmaModeller.h"

//#define PROTREC_DEBUG

class PointMatch {
public:
  typedef Eigen::Vector2d value_type;

  PointMatch(const Eigen::Vector2d& first, const Eigen::Vector2d& second): mFirst(first), mSecond(second) {}

  const Eigen::Vector2d& first() const {
    return mFirst; 
  }

  const Eigen::Vector2d& second() const {
    return mSecond;
  }
  
private:
  Eigen::Vector2d mFirst;
  Eigen::Vector2d mSecond;
};


template<class PixelType>
std::vector<shiken::RegionInfo> createRegions(const vigra::BasicImage<PixelType>& componentsImage, vigra::BasicImage<unsigned>& labelImage, int regionCount, bool secondPass) {
  std::vector<shiken::RegionInfo> result(regionCount);

  /* First pass. */
  for(int y = 0; y < componentsImage.height(); y++) {
    for(int x = 0; x < componentsImage.width(); x++) {
      unsigned label = labelImage(x, y);
      if(label == 0)
        continue;

      shiken::RegionInfo& region = result[label];

      region.setColor(vigra::Converter<PixelType, vigra::RGBValue<vigra::UInt8> >()(componentsImage(x, y)));
      region.addPointPassOne(x, y);

      if(x == 0 || x == componentsImage.width() - 1 || y == 0 || y == componentsImage.height() - 1 || 
        labelImage(x + 1, y) != label || labelImage(x - 1, y) != label || labelImage(x, y + 1) != label || labelImage(x, y - 1) != label)
          region.addPerimeter(1);
    }
  }

  /* Finalize. */
  foreach(shiken::RegionInfo& region, result)
    region.finalizePassOne();

  if(secondPass) {
    /* Second pass. */
    for(int y = 0; y < componentsImage.height(); y++) {
      for(int x = 0; x < componentsImage.width(); x++) {
        unsigned label = labelImage(x, y);
        if(label == 0)
          continue;

        shiken::RegionInfo& region = result[label];

        region.addPointPassTwo(x, y);
      }
    }

    /* Finalize. */
    foreach(shiken::RegionInfo& region, result)
      region.finalizePassTwo();
  }

  return result;
}

int main(int argc, char** argv) {
  using namespace boost::program_options;
  using namespace std;

  time_t t;
  srand(static_cast<int>(time(&t)));

  try {
    vigra::Size2D maxSize;
    vigra::Rect2D barRect;
    int maxErrorPercent;
    bool noLma, drawResults, checkSum;
    string inputFileName, outFileName, keysFileName, patternFileName;
    int minIterations, maxIterations;

    options_description desc("Allowed options");
    desc.add_options()
      ("help",                                                              "Produce help message.")
      ("input,i",          value<string>(&inputFileName),                   "Input file name.")
      ("output,o",         value<string>(&outFileName)->default_value("out.bmp"), 
                                                                            "Output file name.")
      ("keys,k",           value<string>(&keysFileName),                    "Keypoint file name.")
      ("pattern,x",        value<string>(&patternFileName),                 "Pattern file name.")
      ("draw,d",           bool_switch(&drawResults),                       "Draw results in output file")
      ("position,p",       value<vigra::Rect2D>(&barRect)->default_value(vigra::Rect2D(0, 0, 0, 0), "0:0:0:0"), 
                                                                            "Barcode position in input file, in format x:y:w:h.")
      ("size,s",           value<vigra::Size2D>(&maxSize)->default_value(vigra::Size2D(DEFAULT_MAX_SIZE_X, DEFAULT_MAX_SIZE_Y), boost::lexical_cast<string>(DEFAULT_MAX_SIZE_X) + ":" + boost::lexical_cast<string>(DEFAULT_MAX_SIZE_Y)),
                                                                            "Maximal size of an image for keypoint extraction, in format w:h.")
      ("maxerr,m",         value<int>(&maxErrorPercent)->default_value(2),  "Maximal mismatch in reprojected keypoint position relative to image size, in percent.")
      ("nolevmar,l",       bool_switch(&noLma),                             "Don't use Levenberg-Marquardt algorithm for homography optimization.")
      ("checksum,c",       bool_switch(&checkSum),                          "Check mod 10 checksum.")
      ("min-iterations",   value<int>(&minIterations)->default_value(DEFAULT_MIN_ITERATIONS),
                                                                            "Minimal number of iterations.")
      ("max-iterations",   value<int>(&maxIterations)->default_value(DEFAULT_MAX_ITERATIONS),               
                                                                            "Maximal number of iterations.");

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if(vm.count("help") > 0 || inputFileName.empty() || patternFileName.empty() || keysFileName.empty()) {
      cout << "protrec - protocol recognizer, version " << BRT_VERSION << "." << endl;
      cout << endl;
      cout << "USAGE:" << endl;
      cout << "  protrec [options]" << endl;
      cout << endl;
      cout << desc << endl;
      return 1;
    }

    /* Load keypoints. */
    acv::Extract<> extract;
    loadExtract(extract, keysFileName);

    fixNegativeSize(&barRect, extract.width(), extract.height());

    /* Check that barcode lies inside the image. */
    if(!vigra::Rect2D(0, 0, extract.width(), extract.height()).contains(barRect))
      throw logic_error("Specified barcode position lies outside the image boundaries.");

    /* Load images. */
    vigra::BRGBImage patternImage;
    importImage(patternImage, patternFileName);

    vigra::BImage srcImage;
    importImage(srcImage, inputFileName);

    /* Check sizes. */
    if(patternImage.width() != extract.width() || patternImage.height() != extract.height())
      throw logic_error("Sizes of pattern image and destination image differ.");

    /* Match. */
    vigra::BImage newImage(extract.width(), extract.height());
    match(srcImage, maxSize, extract, newImage, maxErrorPercent / 100.0f, !noLma);

    /* Recognize barcode. */
    barcode::ItfCode code = recognize(newImage, barRect, minIterations, maxIterations, checkSum);

    /* Create regions for pattern image. */
    vigra::BasicImage<unsigned> patternLabelImage(patternImage.size(), 0u);
    int patternRegionsCount = 1 + labelImageWithBackground(srcImageRange(patternImage), destImage(patternLabelImage), true, vigra::white<vigra::RGBValue<vigra::UInt8> >());
    std::vector<shiken::RegionInfo> patternRegions = createRegions(patternImage, patternLabelImage, patternRegionsCount, false);

    /* Calculate max area. */
    int maxArea = 0;
    for(int i = 0; i < patternRegionsCount; i++)
      if(patternRegions[i].area() > maxArea)
        maxArea = patternRegions[i].area();

    /* Mask out small regions. */
    for(int y = 1; y < patternImage.height() - 1; y++) {
      for(int x = 1; x < patternImage.width() - 1; x++) {
        unsigned label = patternLabelImage(x, y);
        if(patternRegions[label].area() < maxArea / 2)
          patternImage(x, y) = vigra::white<vigra::RGBValue<vigra::UInt8> >();
      }
    }

    /* Re-label and re-create regions. */
    patternLabelImage.init(0);
    patternRegionsCount = 1 + labelImageWithBackground(srcImageRange(patternImage), destImage(patternLabelImage), true, vigra::white<vigra::RGBValue<vigra::UInt8> >());
    patternRegions = createRegions(patternImage, patternLabelImage, patternRegionsCount, false);

    /* Binarize input image and create regions. */
    kMeansBinarize(newImage, newImage);
    vigra::BasicImage<unsigned> inputLabelImage(newImage.size(), 0u);
    int inputRegionsCount = 1 + labelImageWithBackground(srcImageRange(newImage), destImage(inputLabelImage), true, vigra::white<vigra::UInt8>());
    vector<shiken::RegionInfo> inputRegions = createRegions(newImage, inputLabelImage, inputRegionsCount, false);

    /* Create mapping pattern region index -> has closest input region? */
    vector<unsigned char> patternRegionHasInputRegions(patternRegions.size(), false);
    vector<unsigned> patternRegionForInputRegion(inputRegions.size(), -1);
    for(unsigned i = 1; i < patternRegions.size(); i++) {
      /* Find closest square-like region. */
      int index = -1;
      double minDist = std::numeric_limits<double>::max();
      for(unsigned j = 1; j < inputRegions.size(); j++) {
        double distSqr = (patternRegions[i].centroid() - inputRegions[j].centroid()).squaredMagnitude();
        if(distSqr < minDist && distSqr < maxArea && inputRegions[j].boundingRect().area() > maxArea && inputRegions[j].boundingRect().area() < maxArea * 4) {
          index = j;
          minDist = distSqr;
        }
      }

      if(index != -1) {
        patternRegionHasInputRegions[i] = true;
        vigra::Rect2D boundingRect = inputRegions[index].boundingRect();

        /* Add this region and every region it includes. */
        /*for(unsigned j = 1; j < inputRegions.size(); j++)
          if(boundingRect.contains(inputRegions[j].boundingRect()))
            patternRegionForInputRegion[j] = i;*/

        patternRegionForInputRegion[index] = i;
      }
    }

    /* Find median bounding box area. */
    std::vector<int> inputRegionAreas;
    for(unsigned j = 0; j < inputRegions.size(); j++) {
      int i = patternRegionForInputRegion[j];
      if(i != -1)
        inputRegionAreas.push_back(inputRegions[j].boundingRect().area());
    }
    if(inputRegionAreas.size() == 0)
      throw logic_error("Pattern file does not define any regions.");
    std::nth_element(inputRegionAreas.begin(), inputRegionAreas.begin() + inputRegionAreas.size() / 2, inputRegionAreas.end());
    int medianArea = inputRegionAreas[inputRegionAreas.size() / 2];
    medianArea += 3 * sqrt(static_cast<float>(medianArea));

    /* Filter out big boxes. */
    for(unsigned j = 0; j < inputRegions.size(); j++) {
      int i = patternRegionForInputRegion[j];
      if(i != -1 && inputRegions[j].boundingRect().area() > medianArea)
        patternRegionForInputRegion[j] = -1;
    }

    /* Find bounding rect of all regions. */
    int inputMinX, inputMinY, inputMaxX, inputMaxY;
    inputMinX = inputMinY = std::numeric_limits<int>::max();
    inputMaxX = inputMaxY = std::numeric_limits<int>::min();
    for(unsigned j = 0; j < inputRegions.size(); j++) {
      int i = patternRegionForInputRegion[j];
      if(i != -1) {
        vigra::Rect2D rect = inputRegions[j].boundingRect();
        inputMinX = std::min(inputMinX, rect.left());
        inputMinY = std::min(inputMinY, rect.top());
        inputMaxX = std::max(inputMaxX, rect.right());
        inputMaxY = std::max(inputMaxY, rect.bottom());
      }
    }
    vigra::Rect2D inputRect(inputMinX, inputMinY, inputMaxX, inputMaxY);
    int inputShrink = sqrt(static_cast<float>(medianArea));
    inputRect.addBorder(-inputShrink, -inputShrink);

    /* Filter out central boxes. */
    for(unsigned j = 0; j < inputRegions.size(); j++) {
      int i = patternRegionForInputRegion[j];
      if(i != -1 && inputRect.contains(inputRegions[j].bbCenter()))
        patternRegionForInputRegion[j] = -1;
    }

    /* Build matches. */
    std::vector<PointMatch> pointMatches;
    for(unsigned j = 0; j < inputRegions.size(); j++) {
      int i = patternRegionForInputRegion[j];
      if(i != -1) {
        vigra::Point2D inputCenter = inputRegions[j].bbCenter();
        vigra::Point2D patternCenter = patternRegions[i].bbCenter();
        pointMatches.push_back(PointMatch(Eigen::Vector2d(patternCenter.px(), patternCenter.py()), Eigen::Vector2d(inputCenter.px(), inputCenter.py())));
      }
    }

    /* Match */
    typedef acv::HomographyLmaModeller<PointMatch> Modeller;
    Modeller lmaModeller(pointMatches);
    acv::Lma<Modeller> lma(lmaModeller);
    Modeller::model_type model = lma(Modeller::model_type(Modeller::model_type::Identity()));

#ifdef PROTREC_DEBUG
    vigra::BRGBImage tmp;
    convert(newImage, tmp);
    foreach(PointMatch& match, pointMatches)
      drawLine(tmp, match.first().x(), match.first().y(), match.second().x(), match.second().y(), vigra::RGBValue<vigra::UInt8>(255, 0, 0));
    exportImage(tmp, "marked.png");
#endif

    /* Warp. */
    vigra::BImage warpedNewImage(newImage.size(), vigra::white<vigra::UInt8>());
    warpImageNearestNeightbour(newImage, warpedNewImage, model);
    warpedNewImage.swap(newImage);

    /* Write normalized file if not drawing results. */
    if(!drawResults)
      exportImage(newImage, outFileName);

    /* Prepare to draw results if needed. */
    vigra::BRGBImage resultImage;
    if(drawResults)
      convert(newImage, resultImage);

    /* Rebuild input image. */
    for(int y = 1; y < newImage.height() - 1; y++) {
      for(int x = 1; x < newImage.width() - 1; x++) {
        if(newImage(x, y) == vigra::white<vigra::UInt8>()) {
          patternLabelImage(x, y) = 0;
          patternImage(x, y) = vigra::white<vigra::RGBValue<vigra::UInt8> >();
          continue;
        }
      }
    }

    /* Save old pattern regions. */
    std::map<std::pair<int, int>, std::vector<shiken::RegionInfo> > oldRegionMap;
    for(unsigned i = 1; i < patternRegions.size(); i++) {
      shiken::RegionInfo& region = patternRegions[i];
      oldRegionMap[std::make_pair(region.color().red(), region.color().green())].push_back(region);
    }

    /* Re-label. */
    patternRegions = createRegions(patternImage, patternLabelImage, patternRegionsCount, true);

    /* Construct map. */
    std::map<std::pair<int, int>, std::vector<shiken::RegionInfo> > regionMap;
    for(unsigned i = 1; i < patternRegions.size(); i++) {
      shiken::RegionInfo& region = patternRegions[i];
      if(region.area() > 0)
        regionMap[std::make_pair(region.color().red(), region.color().green())].push_back(region);
    }

    /* Write fist row. */
    cout << code.string() << ";";
    cout << sqrt(arx::sqr(model(0, 0)) + arx::sqr(model(0, 1))) << ";"; /* Model defines a rotation transformation, so here we have sqr(SCALE * sin(ALPHA)) + sqr(SCALE * cos(ALPHA)) = sqr(SCALE). */
    cout << extract.width() << ";";
    cout << extract.height() << ";";
    cout << endl;

    /* Examine regions, classify & output results. */
    typedef std::pair<int, int> key_type;
    int crossCount = 0;
    int crossArea = 0;
    map_foreach(const key_type& key, const std::vector<shiken::RegionInfo>& oldRegions, oldRegionMap) {
      const std::vector<shiken::RegionInfo>& regions = regionMap[key];

      int lowerArea = static_cast<int>(0.05 * maxArea);

      /* Find minimal reasonable area. */
      bool indexFound = false;
      unsigned index = std::numeric_limits<unsigned>::max();
      int minArea = std::numeric_limits<int>::max();
      for(unsigned i = 0; i < regions.size(); i++) {
        if(regions[i].area() < minArea && regions[i].area() > lowerArea) {
          index = i;
          minArea = regions[i].area();
          indexFound = true;
        }
      }

      if(indexFound) {
        /* Check that this is a distinctive match. */
        for(unsigned i = 0; i < regions.size(); i++) {
          double rel = static_cast<double>(regions[i].area()) / minArea;
          if(i != index && rel > 0.5 && rel < 2) {
            indexFound = false;
            break;
          }
        }
      }

      /* Check that it's not a filled square. */
      if(indexFound && minArea > static_cast<int>(0.7 * maxArea))
        indexFound = false;

      /* Compare with other crosses. */
      if(indexFound && crossCount > 5) {
        int meanCrossArea = crossArea / crossCount;

        double rel = static_cast<double>(regions[index].area()) / meanCrossArea;
        if(rel < 0.5 || rel > 2)
          indexFound = false;
      }

      if(indexFound) {
        const shiken::RegionInfo& region = regions[index];
        crossCount++;
        crossArea += region.area();
        cout << 
          static_cast<int>(region.color().red()) << "; " << 
          static_cast<int>(region.color().green()) << "; " << 
          static_cast<int>(region.color().blue()) << "; " << endl;

        if(drawResults)
          foreach(const shiken::RegionInfo& oldRegion, oldRegions)
            if(oldRegion.color() == region.color())
              for(int y = oldRegion.minY(); y < oldRegion.maxY(); y++)
                for(int x = oldRegion.minX(); x < oldRegion.maxX(); x++)
                  resultImage(x, y) = (resultImage(x, y) + vigra::RGBValue<vigra::UInt8>(0, 255, 0)) / 2;
      } else if(drawResults) {
        foreach(const shiken::RegionInfo& oldRegion, oldRegions)
          for(int y = oldRegion.minY(); y < oldRegion.maxY(); y++)
            for(int x = oldRegion.minX(); x < oldRegion.maxX(); x++)
              resultImage(x, y) = (resultImage(x, y) + vigra::RGBValue<vigra::UInt8>(255, 0, 0)) / 2;
      }
    }

    if(drawResults)
      exportImage(resultImage, outFileName);

  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
