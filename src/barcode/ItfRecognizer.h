#ifndef BARCODE_ITF_RECOGNIZER_H
#define BARCODE_ITF_RECOGNIZER_H

#include "config.h"
#include <cassert>
#include <cstdlib> /* for rand() */
#include <cmath>   /* for abs() */
#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/range/algorithm/fill.hpp>
#include <boost/range/numeric.hpp>
#include <boost/algorithm/string/predicate.hpp> /* for boost::starts_with and boost::ends_with */
#include <vigra/stdimage.hxx>
#include <vigra/splineimageview.hxx>
#include <arx/Foreach.h>
#include "ItfEncoding.h"
#include "ItfCode.h"

/**
 * @def DEBUG_BARCODE
 * 
 * Turn debug output on/off.
 */
//#define DEBUG_BARCODE

#ifdef DEBUG_BARCODE
#  include <arx/ext/VigraQt.h>
#endif

namespace barcode {
// -------------------------------------------------------------------------- //
// ItfRecognizer
// -------------------------------------------------------------------------- //
  /**
   * Functor that performs Interleaved 2 of 5 code recognition.
   */
  class ItfRecognizer: public boost::noncopyable {
  public:
    typedef vigra::BImage::value_type value_type;

    ItfRecognizer(const vigra::BImage& img): mImg(img), mView(srcImageRange(img)) {
      assert(img.height() >= 5 && img.width() >= 5);
    };

    /**
     * @param minIterations            Minimal number of iterations to perform,
     *                                 even if solution was found on the
     *                                 first iteration.
     * @param maxIterations            Maximal number of iterations to perform.
     */
    ItfCode operator() (int minIterations, int maxIterations) const {
      std::map<ItfCode, int> counts;
      std::vector<value_type> line;
      std::vector<int> accumulatedLine;
      std::vector<char> binaryLine;
      std::vector<int> hystogram;
      std::vector<int> segments;
      std::vector<char> binarySegments;

#ifdef DEBUG_BARCODE
      vigra::BImage debugImage;
#endif

      for(int i = 0; i < minIterations || (i < maxIterations && counts.size() == 0); i++) {
        accumulatedLine.clear();

        /* Create accumulated scanline. 
         * 
         * The number of lines used increases with iteration number. */
        unsigned lineCount = random(1, 16 + i / 4);
        for(unsigned j = 0; j < lineCount; j++) {
          vigra::Diff2D lineStart(0, random(0, mImg.height()));
          vigra::Diff2D lineEnd(mImg.width() - 1, random(0, mImg.height()));
          scanLine(line, lineStart, lineEnd, 0.5 * (lineEnd - lineStart).magnitude() / mImg.width());

          if(accumulatedLine.size() == 0 || accumulatedLine.size() > line.size())
            accumulatedLine.resize(line.size(), 0);
          for(unsigned k = 0; k < accumulatedLine.size(); k++)
            accumulatedLine[k] += line[k];
        }

        line.resize(accumulatedLine.size());
        for(unsigned k = 0; k < line.size(); k++)
          line[k] = static_cast<value_type>(accumulatedLine[k] / lineCount);

#ifdef DEBUG_BARCODE
        /* Draw line on the debug image. */
        if(debugImage.width() == 0)
          debugImage.resize(accumulatedLine.size(), maxIterations * 2, 0);
        for(unsigned x = 0; x < line.size(); x++)
          debugImage(x, i) = line[x];
#endif

        /* Crop line. 
         * 
         * K-means binarization is currently used for cropping, which is not
         * so good. If we have printing problems on the sides of barcode, then
         * everything will fail.
         * 
         * We cannot use threshold-based binarization here as the black/white 
         * pixel count ratio is not known at this point. */
        cropLine(line);

        ItfCode code;

        /* Try k-means binarization first. */
        kthBinarize(line, binaryLine);
        code = recognize(binaryLine, segments, binarySegments);

        /* If k-means binarization failed, then try threshold-based 
         * binarization. 
         * 
         * At this point it is known that black/white pixel count
         * ratio in the cropped line must equal 0.5. We use slightly larger 
         * random value as printing problems often result in smaller number of
         * black pixels. Additional median filtering helps deal with jpeg
         * ringing and "holes" in black segments. */
        if(code.size() == 0) {
          /* Build hystogram and find threshold value. */
          buildHystogram(line, hystogram, 255);
          unsigned count = 0, targetCount = line.size() * (50 + random(0, 20)) / 100, median;
          for(median = 0; median < hystogram.size(); median++) {
            count += hystogram[median];
            if(count >= targetCount)
              break;
          }

          /* Apply threshold binarization. */
          binaryLine.resize(line.size());
          for(unsigned j = 0; j < line.size(); j++)
            binaryLine[j] = line[j] <= median ? 0 : 1;

          /* Apply random-sized median filter. */
          medianFilter(binaryLine, 3 + 2 * random(0, 3));

#ifdef DEBUG_BARCODE
          /* Draw binarized line on the debug image. */
          for(unsigned x = 0; x < binaryLine.size(); x++)
            debugImage(x, i + maxIterations) = binaryLine[x] * 255;
#endif

          code = recognize(binaryLine, segments, binarySegments);
          if(code.size() == 0)
            continue;
        }

        counts[code]++;
      }

#ifdef DEBUG_BARCODE
      exportImage(debugImage, "DebugBarcode.png");
#endif

      ItfCode result;
      int maxCount = 0;
      map_foreach(const ItfCode& code, const int& count, counts)
        if(count > maxCount)
          boost::tie(result, maxCount) = boost::make_tuple(code, count);
      
      return result;      
    }

  private:
    /**
     * Recognizes barcode.
     * 
     * @param binaryLine[in, out]      Binarized line.
     * @param segments[out]            Buffer for segments.
     * @param binarySegments[out]      Buffer for binarized segments.
     * @returns                        Recognized barcode, or empty barcode if
     *                                 recognition fails.
     */
    static ItfCode recognize(std::vector<char> &binaryLine, std::vector<int> &segments, std::vector<char> &binarySegments) {
      ItfCode result;

      /* Build segment sequence. */
      segments.clear();
      binaryLine.push_back(!binaryLine.back()); /* Add stop. */

      std::size_t pos = 0;
      while(binaryLine[pos] && pos < binaryLine.size()) 
        pos++; /* Skip first white segment. */

      while(pos + 1 < binaryLine.size()) {
        int newPos = pos;
        while(binaryLine[pos] == binaryLine[newPos])
          newPos++;
        segments.push_back(newPos - pos);
        pos = newPos;
      }

      if(segments.size() % 2 == 0)
        segments.pop_back(); /* Skip last white segment. */

      /* Check length. */
      if(segments.size() % 10 != 7)
        return result;

      /* Repeat with median & k-means binarization. */
      kthBinarize(segments, binarySegments, 3.0f / 5.0f, binaryLine.size()); /* 3/5 of all lines are thin. */

      /* Check head & tail. 
        *
        * See note in description for ItfEncoding for some info on what 
        * inverted flag is used for. */
      bool valid;
      bool inverted;
      if(boost::starts_with(binarySegments, ItfEncoding::flatHead) && boost::ends_with(binarySegments, ItfEncoding::flatTail)) {
        valid = true;
        inverted = false;
      } else if(boost::starts_with(binarySegments, ItfEncoding::invertedFlatHead) && boost::ends_with(binarySegments, ItfEncoding::invertedFlatTail)) {
        valid = true;
        inverted = true;
      } else {
        valid = false;
      }
      if(!valid)
        return result;

      /* Build solution. */
      ItfCode code;
      for(std::size_t i = 4; i + 9 < binarySegments.size(); i += 10) {
        int a = decode(inverted, binarySegments.begin() + i);
        int b = decode(inverted, binarySegments.begin() + i + 1);
        if(a == -1 || b == -1) {
          result.clear();
          break;
        }
        result.addDigit(a);
        result.addDigit(b);
      }

      return result;
    }

    /**
     * @returns                        Median of the given values.
     */
    static value_type median(value_type a, value_type b, value_type c) {
      if(a > b)
        return (b > c) ? b : ((a > c) ? c : a);
      else
        return (b < c) ? b : ((a < c) ? c : a);
    }

    /**
     * Performs median filtering with the given window size of the given
     * 1-dimensional binary image.
     * 
     * @param binaryLine[in, out]      Binary image to filter.
     * @param size                     Window size of the median filter. Must
     *                                 be an odd number.
     */
    static void medianFilter(std::vector<char> &binaryLine, unsigned size) {
      assert(size % 2 == 1);

      if(binaryLine.size() < size)
        return;

      unsigned whiteCount = 0;
      for(unsigned i = 0; i < size; i++)
        whiteCount += binaryLine[i];

      for(unsigned i = size / 2 + 1; true; i++) {
        if(whiteCount > size - whiteCount)
          binaryLine[i] |= 0x2;

        if(i + 1 + size / 2 >= binaryLine.size())
          break;

        whiteCount += binaryLine[i + 1 + size / 2];
        whiteCount -= binaryLine[i - size / 2] >> 1;
      }

      for(unsigned i = size / 2 + 1; i + 1 + size / 2 < binaryLine.size(); i++)
        binaryLine[i] >>= 1;
    }

    /**
     * @returns                        Random number in range [lo, hi).
     */
    static int random(int lo, int hi) {
      /* On gcc RAND_MAX == INT_MAX, so we need to cast it to long long. */
      return static_cast<int>(lo + static_cast<long long>(hi - lo) * rand() / (static_cast<long long>(RAND_MAX) + 1));
    }

    template<class T>
    static void kthBinarize(const std::vector<T> &line, std::vector<char> &binaryLine, float k = 0.5f) {
      kthBinarize(line, binaryLine, k, (1 << (8 * sizeof(T))) - 1);
    }

    /**
     * Performs k-means binarization of the given set of values.
     *
     * Centers of 0- and 1-clusters will be initialized with values that
     * are equally-spaced from n-th order statistic of the given set of values,
     * where n equals the size of the given set of values times k.
     *
     * @param values                   Set of values to binarize.
     * @param[out] binaryValues        Output binary mask.
     * @param k                        Value in range [0, 1] that determines
     *                                 the initial centers of 0- and 1-clusters.
     * @param maxValue                 Maximal value of a set element.
     */
    template<class T>
    static void kthBinarize(const std::vector<T> &values, std::vector<char> &binaryValues, float k, int maxValue) {
      assert(k >= 0 && k <= 1);

      std::vector<int> hystogram;

      buildHystogram(values, hystogram, maxValue);
      int kth = nthElement(hystogram, static_cast<int>(values.size() * k));
      binarize(values, binaryValues, kth - 1, kth + 1);
    }

    /**
     * Binarizes the given colored segment and crops its "fields", i.e. the
     * sequences at the beginning and at the end of the segment that end up
     * in the binary cluster.
     *
     * @param line[in, out]            Colored segment.
     */
    void cropLine(std::vector<value_type> &line) const {
      std::vector<char> binaryLine;
      kthBinarize(line, binaryLine);

      /* Crop line. */
      std::size_t lCrop = 0;
      while(binaryLine[lCrop] && lCrop < binaryLine.size())
        lCrop++;
      std::size_t rCrop = 0;
      while(binaryLine[binaryLine.size() - rCrop - 1] && rCrop < binaryLine.size())
        rCrop++;

      if(lCrop + rCrop >= line.size()) {
        line.clear();
      } else {
        line.erase(line.end() - rCrop - 1, line.end());
        line.erase(line.begin(), line.begin() + lCrop);
      }
    }

    /**
     * Extracts a colored segment from the image.
     *
     * @param out[out]                 Vector to write the segment color 
     *                                 information to.
     * @param v0                       Coordinate of the start of the segment.
     * @param v0                       Coordinate of the end of the segment.
     * @param step                     Number of image pixels per segment pixel.
     */
    void scanLine(std::vector<value_type>& out, vigra::Diff2D v0, vigra::Diff2D v1, double step) const {
      assert(mImg.isInside(v0) && mImg.isInside(v1));

      out.clear();
      double length = (v1 - v0).magnitude();
      double dx = (v1.x - v0.x) / length;
      double dy = (v1.y - v0.y) / length;
      
      double x0 = v0.x, y0 = v0.y;
      double t = 0;
      do {
        out.push_back(mView(x0 + dx * t, y0 + dy * t));
        t += step;
      } while(t < length);
    }

    /**
     * Binarizes the given set of values using k-means binarization.
     *
     * @param values                   Set of values to binarize.
     * @param out[out]                 Output binary mask.
     * @param a                        Initial center of 0-cluster.
     * @param b                        Initial center of 1-cluster.
     */
    template<class T>
    static void binarize(const std::vector<T>& values, std::vector<char>& out, int a, int b) {
      out.resize(values.size());

      /* Adjust means. */
      while(true) {
        assert(b >= a);

        int aNew = 0, bNew = 0, bCount = 0, aCount = 0;
        foreach(T value, values) {
          if(b - value < value - a) {
            bCount++;
            bNew += value;
          } else {
            aCount++;
            aNew += value;
          }
        }
        aNew = aCount != 0 ? aNew / aCount : a;
        bNew = bCount != 0 ? bNew / bCount : b;

        if(std::abs(a - aNew) < 1 && std::abs(b - bNew) < 1)
          break;
        
        a = aNew;
        b = bNew;
      }
      
      /* Classify. */
      for(std::size_t i = 0; i < values.size(); i++)
        out[i] = (b - values[i] < values[i] - a) ? 1 : 0;
    }

    /**
     * Constructs a hystogram for the given set of values.
     *
     * @param values                   Set of values to constryct hystogram for.
     * @param out[out]                 Hystogram.
     * @param maxValue                 Maximal value of a set element.
     */
    template<class T>
    static void buildHystogram(const std::vector<T>& values, std::vector<int>& out, int maxValue) {
      out.resize(maxValue + 1);
      
      boost::fill(out, 0);
      foreach(T value, values)
        out[value]++;
    }

    /**
     * Finds nth smallest element in a set given its hystogram.
     *
     * @param hystogram                Hystogram of a set.
     * @param n                        Number of the smallest element to find.
     * @returns                        Nth smallest element.
     */
    static int nthElement(std::vector<int>& hystogram, int n) {
      int sum = 0;
      std::size_t element;
      for(element = 0; element < hystogram.size(); element++)
        if((sum += hystogram[element]) >= n)
          return element;
      
      unreachable();
    }

    /**
     * Decodes the binary sequence starting at a given position into a digit.
     *
     * @returns                        Decoded digit, or -1 in case of an
     *                                 invalid input.
     */
    template<class RandomAccessIterator>
    static int decode(bool inverted, RandomAccessIterator pos) {
      /* GCC generates incorrect (?) code if this is written in one values, 
       * without temporaries. */
      const int *decoding = inverted ? ItfEncoding::invertedDecoding : ItfEncoding::decoding;
      int index = *pos * 16 + *(pos + 2) * 8 + *(pos + 4) * 4 + *(pos + 6) * 2 + *(pos + 8);
      return decoding[index];
    }

    const vigra::BImage& mImg;
    const vigra::SplineImageView<3, value_type> mView;
  };

} // namespace barcode

#endif // BARCODE_ITF_RECOGNIZER_H
