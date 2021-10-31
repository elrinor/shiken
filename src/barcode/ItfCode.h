#ifndef BARCODE_ITF_CODE_H
#define BARCODE_ITF_CODE_H

#include "config.h"
#include <cassert>
#include <vector>
#include <boost/operators.hpp>
#include <vigra/stdimage.hxx>
#include <vigra/basicimageview.hxx>
#include <vigra/resizeimage.hxx>
#include <arx/Foreach.h>
#include "ItfEncoding.h"
#include "DigitImages.h"
#include "ItfBar.h"

namespace barcode {
// -------------------------------------------------------------------------- //
// ItfCode
// -------------------------------------------------------------------------- //
  /**
   * Interleaved 2 of 5 code.
   */
  class ItfCode: public boost::less_than_comparable1<ItfCode> {
  public:
    /**
     * Default constructor.
     *
     * Constructs empty barcode.
     */
    ItfCode() {}

    /**
     * Constructor.
     *
     * @param s                        String representation of the barcode,
     *                                 must contain digit characters only.
     */
    ItfCode(const std::string& s) {
      addDigits(s);
    }

    /**
     * @param digit                    Digit to add to this barcode.
     */
    void addDigit(int digit) {
      assert(digit >= 0 && digit < 10);

      mSequence.push_back(digit);
    }

    /**
     * @param s                        String representation of a digit 
     *                                 sequence to add to the barcode.
     */
    void addDigits(const std::string& s) {
      mSequence.reserve(mSequence.size() + s.size());

      foreach(char c, s)
        addDigit(c - '0');
    }

    /**
     * @returns                        Checksum of this barcode as specified
     *                                 in the standard.
     */
    int mod10CheckSum() const {
      int odds = 0, evens = 0;

      for(std::size_t i = 0; i < mSequence.size(); i++)
        ((i + 1) % 2 == 0 ? evens : odds) += mSequence[i];

      /* Algorithm is as follows:
       * 
       * 1. Add up the numbers at odd positions.
       * 2. Multiply the sum by 3.
       * 3. Add up the numbers at even positions.
       * 4. Determine the number that when added to the sum, will produce a multiple of 10. */
      return (10 - (odds * 3 + evens) % 10) % 10;
    }

    /**
     * Adds standard checksum to this barcode.
     */ 
    void addMod10CheckSum() {
      if(size() % 2 == 0)
        addDigit(0);
      addDigit(mod10CheckSum());
    }

    /**
     * @returns                        The size of this barcode, in digits.
     */
    int size() const {
      return mSequence.size();
    }

    /**
     * Clears this barcode.
     */
    void clear() {
      mSequence.clear();
    }

    /** Creates barcode image.
     * 
     * @param[in, out] img             image to draw on.
     * @param barWidth                 width in pixels of a single narrow bar.
     * @param barHeight                height of bar area in pixels.
     * @param midSpace                 height of empty area between barcode and label in pixels.
     * @param labelHeight              height of label in pixels. */
    void createImage(vigra::BImage& img, int barWidth, int barHeight, int midSpace, int labelHeight) const {
      assert(mSequence.size() % 2 == 0);
      assert(barHeight > 0 && midSpace >= 0 && labelHeight >= 0);

      /* Adjust input params. */
      if(labelHeight < 2) {
        labelHeight = 0;
        midSpace = 0;
      }

      /* Width of barcode. */
      int barCodeWidth = barWidth * (
        sideSkip() * 2 + 
        2 * widthMultiplier(ItfEncoding::head[0]) +
        widthMultiplier(ItfEncoding::encoding[0]) * mSequence.size() +
        2 * widthMultiplier(ItfEncoding::tail[0])
      );

      /* Width of label. */
      int labelWidth = 
        (labelHeight == 0) ? 0 : mSequence.size() * (DigitImages::image(0).width() * labelHeight / DigitImages::image(0).height());

      /* Init. */
      img.resize(std::max(labelWidth, barCodeWidth), barHeight + midSpace + labelHeight);
      img.init(255);

      /* Draw. */
      int x = (img.width() - barCodeWidth) / 2 + barWidth * sideSkip();
      foreach(ItfBar bar, bars())
        x = drawBar(img, bar.isWhite() ? 255 : 0, bar.isThick() ? 1 : 0, barWidth, barHeight, x);

      /* Draw label if needed. */
      if(labelHeight == 0)
        return;
      
      x = (img.width() - labelWidth) / 2;
      foreach(int digit, mSequence) {
        const vigra::BImage& digitImage = DigitImages::image(digit);
        vigra::Size2D size = vigra::Size2D(digitImage.width() * labelHeight / digitImage.height(), labelHeight);
        resizeImageLinearInterpolation(srcImageRange(digitImage), destImageRange(img, vigra::Rect2D(vigra::Point2D(x, barHeight + midSpace), size)));
        x += size.width();
      }
    }

    /**
     * @returns                        A sequence of bars constituting this 
     *                                 barcode.
     */
    std::vector<ItfBar> bars() const {
      std::vector<ItfBar> result;

      addInterleavedCode(result, ItfEncoding::head[0], ItfEncoding::head[1]);
      for(std::size_t i = 0; i < mSequence.size(); i += 2) /* v- Fix for GCC warning "array subscript has type char" -v */
        addInterleavedCode(result, ItfEncoding::encoding[static_cast<int>(mSequence[i])], ItfEncoding::encoding[static_cast<int>(mSequence[i + 1])]);
      addInterleavedCode(result, ItfEncoding::tail[0], ItfEncoding::tail[1]);

      return result;
    }

    /**
     * @returns                        String representation of this barcode.
     */
    std::string string() const {
      std::string s;
      s.reserve(mSequence.size());
      foreach(char c, mSequence)
        s.push_back(c + '0');
      return s;
    }

    bool operator< (const ItfCode& other) const {
      return mSequence < other.mSequence;
    }

  private:
    static int drawBar(vigra::BImage& img, int color, int bit, int barWidth, int barHeight, int x) {
      int w = barWidth * widthMultiplier(bit);
      vigra::BasicImageView<vigra::UInt8>(&img[0][x], vigra::Size2D(w, barHeight), img.width()).init(color);
      return x + w;
    }

    template<int arraySize>
    static void addInterleavedCode(std::vector<ItfBar>& result, const int (&bbits)[arraySize], const int (&wbits)[arraySize]) {
      for(int i = 0; i < arraySize; i++) {
        result.push_back(ItfBar(!!bbits[i], false));
        result.push_back(ItfBar(!!wbits[i], true));
      }
    }

    static int widthMultiplier(int bit) {
      assert((bit & 0xFFFFFFFE) == 0);

      return (bit == 0) ? 1 : 3;
    }

    static int sideSkip() {
      return 10;
    }

    template<int size>
    static int widthMultiplier(const int (&bits)[size]) {
      int result = 0;
      foreach(int bit, bits)
        result += widthMultiplier(bit);
      return result;
    }

    std::vector<char> mSequence;
  };

} // namespace barcode

#endif // BARCODE_ITF_CODE_H
