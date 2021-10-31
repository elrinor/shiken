#ifndef BARCODE_DIGIT_IMAGES_H
#define BARCODE_DIGIT_IMAGES_H

#include "config.h"
#include <boost/preprocessor.hpp>
#include <vigra/stdimage.hxx>
#include <arx/Utility.h>

namespace barcode {
  namespace detail {
    /* See end of file for vertical repetition macro. 
     * Note that angle brackets here is a workaround. Source root must be in include path. */
#define BOOST_PP_ITERATION_LIMITS (0, 9)
#define BOOST_PP_FILENAME_1 <barcode/DigitImages.h>
#include BOOST_PP_ITERATE()
  } // namespace detail

// -------------------------------------------------------------------------- //
// DigitImages
// -------------------------------------------------------------------------- //
  class DigitImages {
  public:
    static const vigra::BImage& image(int i) {
      switch(i) {
#define DIGIT_IMAGES_CASE(r, _, e)                                              \
      case e: return detail::BOOST_PP_CAT(digit_, e)::Image::image();
      BOOST_PP_SEQ_FOR_EACH(DIGIT_IMAGES_CASE, ~, (0)(1)(2)(3)(4)(5)(6)(7)(8)(9))
#undef DIGIT_IMAGES_CASE
      default: unreachable();
      }
    }
  };

} // namespace barcode

#endif // BARCODE_DIGIT_IMAGES_H

/* Vertical repetition macro follows. */

#ifdef BOOST_PP_IS_ITERATING

#define gencimg BOOST_PP_CAT(digit_, BOOST_PP_ITERATION())
#include BOOST_PP_STRINGIZE(../res/BOOST_PP_ITERATION().bmp.i)
#undef gencimg

#endif // BOOST_PP_IS_ITERATING
