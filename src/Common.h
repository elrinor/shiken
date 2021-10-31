#ifndef COMMON_H
#define COMMON_H

#include "config.h"
#include <fstream>
#include <algorithm> /* for std::max() */
#include <exception> /* for std::logic_error */
#include <vector>
#include <boost/lexical_cast.hpp>
#include <arx/ext/Vigra.h>
#include <arx/ext/Qt.h>
#include <arx/ext/VigraQt.h>
#include "acv/Extract.h"
#include "acv/Extractor.h"
#include "acv/Matcher.h"
#include "acv/Lma.h"
#include "acv/CollageLmaModeller.h"
#include "acv/CollageRansacModeller.h"
#include "barcode/ItfRecognizer.h"
#include "ImageUtils.h"

typedef acv::CollageRansacModeller<acv::Match> RansacModeller;
typedef acv::CollageLmaModeller<acv::Match> LmaModeller;
typedef RansacModeller::model_type RansacModel;

/**
 * Extracts keypoints from a given image.
 * 
 * @param scrImage                     Image to extract keypoints from.
 * @param maxKeyImageSize              Maximal size of an image to extract
 *                                     keypoints from. Source image will be 
 *                                     resized before keypoint extraction to
 *                                     fit into this size.
 * @param[out] extract                 Keypoint extract.
 */
template<class PixelType, class VigraAlloc, class Allocator>
void extractKeypoints(const vigra::BasicImage<PixelType, VigraAlloc>& srcImage, const vigra::Size2D& maxKeyImageSize, acv::Extract<Allocator>& extract) {
  typedef vigra::BasicImage<float, typename VigraAlloc::template rebind<float>::other> Image;

  /* Resize if needed. */
  boost::scoped_ptr<Image> imageHolder;
  const Image* pKeyImage = NULL;
  float scale = std::max(1.0f, std::max(static_cast<float>(srcImage.width()) / maxKeyImageSize.width(), static_cast<float>(srcImage.height()) / maxKeyImageSize.height()));
  if(scale > 1) {
    using std::swap;

    imageHolder.reset(new Image(srcImage.width() / scale, srcImage.height() / scale));
    resizeImageLinearInterpolation(srcImageRange(srcImage, vigra::ConvertingAccessor<PixelType, float>()), destImageRange(*imageHolder));
    pKeyImage = imageHolder.get();
  } else {
    imageHolder.reset(new Image());
    pKeyImage = &convert_nocopy(srcImage, *imageHolder);
  }
  const Image& keyImage = *pKeyImage;

  /* Extract keypoints from input image. */
  acv::Extractor()(keyImage, INITIAL_SMOOTHNESS / scale, scale, extract);

  /* Check number of extracted keypoints. */
  if(extract.keypoints().size() < MIN_KEYPOINTS_PER_IMAGE) {
    throw std::logic_error(
      "Too little keypoints found in image: " + boost::lexical_cast<std::string>(extract.keypoints().size()) + 
      " found while at least " + boost::lexical_cast<std::string>(MIN_KEYPOINTS_PER_IMAGE) + " are required."
      );
  }
}

/**
 * Matches the given image against the given keypoint extract and aligns it
 * correspondingly.
 *
 * @param scrImage                     Image to match.
 * @param maxKeyImageSize              Maximal size of an image to extract
 *                                     keypoints from. Source image will be 
 *                                     resized before keypoint extraction to
 *                                     fit into this size.
 * @param extract                      Keypoint extract to match the source
 *                                     image to.
 * @param[out] outImage                Aligned image.
 * @param maxRansacError               Maximal RANSAC error used for keypoint
 *                                     match filtering.
 * @param useLma                       Perform transformation optimization with 
 *                                     Levenberg-Marquardt after initial
 *                                     estimation via RANSAC?
 */
template<class PixelType, class VigraAlloc, class Allocator>
RansacModel match(
  const vigra::BasicImage<PixelType, VigraAlloc>& srcImage, 
  const vigra::Size2D& maxKeyImageSize,
  const acv::Extract<Allocator>& extract, 
  vigra::BasicImage<PixelType, VigraAlloc>& outImage, 
  double maxRansacError, 
  bool useLma
) {
  /* Check number of input keypoints. */
  if(extract.keypoints().size() < MIN_KEYPOINTS_PER_IMAGE) {
    throw std::logic_error(
      "Too little keypoints were provided: " + boost::lexical_cast<std::string>(extract.keypoints().size()) + 
      " while at least " + boost::lexical_cast<std::string>(MIN_KEYPOINTS_PER_IMAGE) + " are required."
    );
  }

  /* Extract keypoints from input image. */
  acv::Extract<> newExtract;
  extractKeypoints(srcImage, maxKeyImageSize, newExtract);

  /* Match. */
  std::vector<acv::Match> matches;
  acv::Matcher<RansacModeller> matcher(RansacModeller(extract.width(), extract.height()), maxRansacError, MIN_MATCHES, MAX_MATCHES);
  if(!matcher(extract.keypoints(), newExtract.keypoints(), matches))
    throw std::logic_error("Image did not match to the keypoints provided.");

  /* Optimize if needed. */
  RansacModel model = matcher.bestModel();
  if(useLma)
    model = acv::Lma<LmaModeller>(LmaModeller(matches))(model);

  /* Warp. */
  outImage.resize(extract.width(), extract.height());
  outImage.init(vigra::white<PixelType>());
  warpImage(srcImage, outImage, model);

  /* Ok. */
  return model;
}

/**
 * Recognizes barcode in a given image.
 *
 * @param img                          Image that contains the barcode.
 * @param barcodePos                   Position of the barcode in an image.
 * @param minIterations                Minimal number of recognition iterations.
 * @param maxIterations                Maximal number of recognition iterations.
 * @param checkSum                     Check ITF checksum?
 * @returns                            Recognized barcode.
 */
template<class PixelType, class Alloc>
barcode::ItfCode recognize(const vigra::BasicImage<PixelType, Alloc>& img, const vigra::Rect2D& barcodePos, int minIterations, int maxIterations, bool checkSum) {
  vigra::BImage codeImage(barcodePos.size());
  copyImage(srcImageRange(img, barcodePos, vigra::ConvertingAccessor<PixelType, vigra::UInt8>()), destImage(codeImage));

  barcode::ItfCode result = barcode::ItfRecognizer(codeImage)(minIterations, maxIterations);
  if(result.size() == 0)
    throw std::logic_error("Could not recognize barcode.");
  if(checkSum && result.mod10CheckSum() != 0)
    throw std::logic_error("Wrong checksum: " + boost::lexical_cast<std::string>(result.mod10CheckSum()) + " instead of 0 for barcode " + result.string());
  return result;
}

/**
 * Loads keypoint extract from a file.
 * 
 * @param[out] extract                 Extract to load.
 * @param fileName                     Filename of the extract.
 */
template<class Allocator>
void loadExtract(acv::Extract<Allocator>& extract, std::string fileName) {
  std::ifstream f(fileName.c_str());
  f >> extract;
  if(f.fail())
    throw std::logic_error("Invalid keypoint file format.");
  f.close();
}

#endif // COMMON_H
