#ifndef SHIKEN_REGION_INFO_H
#define SHIKEN_REGION_INFO_H

#include "config.h"
#include <cmath>     /* for sqrt() */
#include <algorithm> /* for std::min() & std::max() */
#include <limits>
#include <arx/Utility.h>

namespace shiken {
// -------------------------------------------------------------------------- //
// RegionInfo
// -------------------------------------------------------------------------- //
  class RegionInfo {
  public: 
    RegionInfo(): 
      mArea(0), 
      mPerimeter(0), 
      mMaxX(std::numeric_limits<int>::min()), 
      mMaxY(std::numeric_limits<int>::min()), 
      mMinX(std::numeric_limits<int>::max()), 
      mMinY(std::numeric_limits<int>::max()),
      mM11(0), 
      mM20(0), 
      mM02(0), 
      mCentroid(0, 0)
    {
#ifdef _DEBUG
      mPassOneFinalized = false;
      mPassTwoFinalized = false;
#endif
    }

    const vigra::RGBValue<vigra::UInt8>& color() const {
      return mColor;
    }

    void setColor(const vigra::RGBValue<vigra::UInt8>& color) {
      mColor = color;
    }

    int area() const {
      return mArea;
    }

    vigra::Rect2D boundingRect() const {
      return vigra::Rect2D(mMinX, mMinY, mMaxX + 1, mMaxY + 1);
    }

    int maxX() const {
      return mMaxX;
    }

    int maxY() const {
      return mMaxY;
    }

    int minX() const {
      return mMinX;
    }

    int minY() const {
      return mMinY;
    }

    int perimeter() const {
      return mPerimeter;
    }

    void addPerimeter(int perimeterDelta) {
      mPerimeter += perimeterDelta;
    }

    const vigra::Point2D& centroid() const {
      assert(mPassOneFinalized);
      return mCentroid;
    }

    vigra::Point2D bbCenter() const {
      assert(mPassOneFinalized);
      return vigra::Point2D((mMinX + mMaxX) / 2, (mMinY + mMaxY) / 2);
    }

    void addPointPassOne(int x, int y) {
      assert(!mPassOneFinalized);
      mCentroid += vigra::Point2D(x, y);
      mMaxX = std::max(mMaxX, x);
      mMinX = std::min(mMinX, x);
      mMaxY = std::max(mMaxY, y);
      mMinY = std::min(mMinY, y);
      mArea++;
    }

    void finalizePassOne() {
      assert(!mPassOneFinalized);
      if(mArea != 0)
        mCentroid /= mArea;
#ifdef _DEBUG
      mPassOneFinalized = true;
#endif
    }

    double m02() const {
      return mM02;
    }

    double m20() const {
      return mM20;
    }

    double m11() const {
      return mM11;
    }

    void addPointPassTwo(int x, int y) {
      assert(mPassOneFinalized && !mPassTwoFinalized);
      
      double dx = x - mCentroid.px();
      double dy = y - mCentroid.py();

      mM11 += dx * dy;
      mM20 += arx::sqr(dx);
      mM02 += arx::sqr(dy);
    }

    void finalizePassTwo() {
      assert(mPassOneFinalized && !mPassTwoFinalized);
      mM11 /= mArea;
      mM20 /= mArea;
      mM02 /= mArea;
#ifdef _DEBUG
      mPassTwoFinalized = true;
#endif
    }

    double elongation() {
      double root = sqrt(arx::sqr(mM20 - mM02) + 4 * arx::sqr(mM11));
      return 
        (mM02 + mM20 + root) / (mM02 + mM20 - root);
    }

    double compactness() {
      return arx::sqr(static_cast<double>(mPerimeter)) / mArea;
    }

  private:
    vigra::RGBValue<vigra::UInt8> mColor;
    int mArea;
    int mPerimeter;
    int mMaxX, mMaxY, mMinX, mMinY;
    double mM11, mM20, mM02;
    vigra::Point2D mCentroid;
#ifdef _DEBUG
    bool mPassOneFinalized;
    bool mPassTwoFinalized;
#endif
  };

} // namespace shiken

#endif // SHIKEN_REGION_INFO_H
