#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include "config.h"
#include <fstream>
#include <Eigen/Dense>
#include <vigra/stdimage.hxx>
#include <vigra/affinegeometry.hxx>
#include <arx/ext/Vigra.h>
#include "acv/Keypoint.h"

template<class PixelType, class Alloc>
void affineWarpImage(const vigra::BasicImage<PixelType, Alloc>& src, vigra::BasicImage<PixelType, Alloc>& dst, const Eigen::Transform2d& srcToDstTransform) {
  /* See important note in warpImage(). */
  Eigen::Transform2d dstToSrcTransform = Eigen::Transform2d(srcToDstTransform.matrix().lu().inverse());
  assert((srcToDstTransform.matrix() * dstToSrcTransform.matrix()).isIdentity());

  vigra::linalg::Matrix<double> vigraTransform(3, 3);
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      vigraTransform[vigra::linalg::Matrix<double>::difference_type(i, j)] = dstToSrcTransform(i, j);
  affineWarpImage(vigra::SplineImageView<3, PixelType>(srcImageRange(src)), destImageRange(dst), vigraTransform);
}

template<class PixelType, class Alloc>
void warpImage(const vigra::BasicImage<PixelType, Alloc>& src, vigra::BasicImage<PixelType, Alloc>& dst, const Eigen::Transform2d& srcToDstTransform) {
  /* Important!
   * We are doing some strange manipulations with lu() because fixed-sized matrices
   * use so-called 'optimized' code paths for inverse() method. It seems that in the process
   * of optimization numerical stability has been optimized away o_O. */
  Eigen::Transform2d dstToSrcTransform = Eigen::Transform2d(srcToDstTransform.matrix().lu().inverse());
  assert((srcToDstTransform.matrix() * dstToSrcTransform.matrix()).isIdentity());

  vigra::SplineImageView<3, PixelType> spline(srcImageRange(src));
  double y = 0.0f;
  for(int iy = 0; y < dst.height(); iy++, y++) {
    double x = 0.0f;
    for(int ix = 0; x < dst.width(); ix++, x++) {
      Eigen::Vector2d v = dstToSrcTransform * Eigen::Vector2d(x, y);
      if(spline.isInside(v[0], v[1]))
        dst(x, y) = spline(v[0], v[1]);
    }
  }
}

template<class PixelType, class Alloc>
void warpImageNearestNeightbour(const vigra::BasicImage<PixelType, Alloc>& src, vigra::BasicImage<PixelType, Alloc>& dst, const Eigen::Transform2d& srcToDstTransform) {
  /* See important note in warpImage(). */
  Eigen::Transform2d dstToSrcTransform = Eigen::Transform2d(srcToDstTransform.matrix().lu().inverse());
  assert((srcToDstTransform.matrix() * dstToSrcTransform.matrix()).isIdentity());

  double y = 0.0f;
  for(int iy = 0; y < dst.height(); iy++, y++) {
    double x = 0.0f;
    for(int ix = 0; x < dst.width(); ix++, x++) {
      Eigen::Vector2d vd = dstToSrcTransform * Eigen::Vector2d(x, y);
      Eigen::Vector2i v = Eigen::Vector2i(static_cast<int>(vd[0] + 0.5), static_cast<int>(vd[1] + 0.5));
      if(src.isInside(vigra::Diff2D(v[0], v[1])))
        dst(x, y) = src(v[0], v[1]);
    }
  }
}


template<class KeyPointForwardCollection, class PixelType, class Alloc>
void markKeypoints(KeyPointForwardCollection& keyPoints, vigra::BasicImage<PixelType, Alloc>& image, const PixelType& color) {
  foreach(acv::Keypoint* keyPoint, keyPoints) {
    float scale = keyPoint->scale();
    float angle = -keyPoint->angle();
    float x1 = keyPoint->x();
    float y1 = keyPoint->y();
    float x2 = x1 + 5.0f * scale * cos(angle);
    float y2 = y1 + 5.0f * scale * sin(angle);
    drawLine(image, (int) x1, (int) y1, (int) x2, (int) y2, color);

    float x3, y3;
    x3 = x2 + 1.0f * scale * cos(angle - M_PI * 0.75f);
    y3 = y2 + 1.0f * scale * sin(angle - M_PI * 0.75f);
    drawLine(image, (int) x2, (int) y2, (int) x3, (int) y3, color);

    x3 = x2 + 1.0f * scale * cos(angle + M_PI * 0.75f);
    y3 = y2 + 1.0f * scale * sin(angle + M_PI * 0.75f);
    drawLine(image, (int) x2, (int) y2, (int) x3, (int) y3, color);
  }
}

#endif // IMAGE_UTILS_H
