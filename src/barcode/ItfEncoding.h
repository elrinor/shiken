#ifndef BARCODE_ITF_ENCODING_H
#define BARCODE_ITF_ENCODING_H

#include "config.h"

namespace barcode {
  namespace detail {
// -------------------------------------------------------------------------- //
// GenericItfEncoding
// -------------------------------------------------------------------------- //
    /**
     * Storage class for Interleaved 2 of 5 encoding and decoding tables.
     *
     * Templated to define everything in a header file.
     *
     * @note There was a bug in the implementation of the Java-side barcode 
     * generator that caused inversion of bar widths (wide bars became thin, 
     * and this bars became wide). The bug was soon discovered, but a number
     * of invalid barcodes has already been printed. To recognize them, 
     * inverted encoding was introduced.
     */
    template<class Tag>
    class GenericItfEncoding {
    public:
      /**
       * Encoding of all digits. Zero for narrow bar, one for wide.
       * The first index is the digit.
       */
      static const int encoding[10][5];

      /**
       * Inverted encoding. Narrow and wide bars are swapped.
       */
      static const int invertedEncoding[10][5];

      /**
       * Decoding table. Five bits of the index into this table describe
       * the bar sequence - 1 for thick bars, 0 for thin ones.
       */
      static const int decoding[32];

      /**
       * Decoding table for inverted encoding.
       */
      static const int invertedDecoding[32];
      
      /**
       * Encoding for head elements. The first index is the color, 0 for black,
       * 1 for white.
       */
      static const int head[2][2];

      /**
       * Encoding for tail elements.
       */
      static const int tail[2][2];

      /**
       * Encoding for the flattened head (with black and white bars interleaved
       * into a single array).
       */
      static const int flatHead[4];

      /**
       * Encoding for inverted flattened head.
       */
      static const int invertedFlatHead[4];

      /**
       * Encoding for the flattened tail.
       */
      static const int flatTail[3];

      /**
       * Encoding for inverted flattened tail.
       */
      static const int invertedFlatTail[3];
    };

    template<class Tag> 
    const int GenericItfEncoding<Tag>::encoding[10][5] = {
      {0, 0, 1, 1, 0}, /* 0 */
      {1, 0, 0, 0, 1}, /* 1 */
      {0, 1, 0, 0, 1}, /* 2 */
      {1, 1, 0, 0, 0}, /* 3 */
      {0, 0, 1, 0, 1}, /* 4 */
      {1, 0, 1, 0, 0}, /* 5 */
      {0, 1, 1, 0, 0}, /* 6 */
      {0, 0, 0, 1, 1}, /* 7 */
      {1, 0, 0, 1, 0}, /* 8 */
      {0, 1, 0, 1, 0}  /* 9 */
    };

    template<class Tag> 
    const int GenericItfEncoding<Tag>::invertedEncoding[10][5] = {
      {1, 1, 0, 0, 1}, /* 0 */
      {0, 1, 1, 1, 0}, /* 1 */
      {1, 0, 1, 1, 0}, /* 2 */
      {0, 0, 1, 1, 1}, /* 3 */
      {1, 1, 0, 1, 0}, /* 4 */
      {0, 1, 0, 1, 1}, /* 5 */
      {1, 0, 0, 1, 1}, /* 6 */
      {1, 1, 1, 0, 0}, /* 7 */
      {0, 1, 1, 0, 1}, /* 8 */
      {1, 0, 1, 0, 1}  /* 9 */
    };

    template<class Tag> 
    const int GenericItfEncoding<Tag>::decoding[32] = {
      /* 00000 */ -1,
      /* 00001 */ -1,
      /* 00010 */ -1,
      /* 00011 */  7,
      /* 00100 */ -1,
      /* 00101 */  4,
      /* 00110 */  0,
      /* 00111 */ -1,
      /* 01000 */ -1,
      /* 01001 */  2,
      /* 01010 */  9,
      /* 01011 */ -1,
      /* 01100 */  6,
      /* 01101 */ -1,
      /* 01110 */ -1,
      /* 01111 */ -1,
      /* 10000 */ -1,
      /* 10001 */  1,
      /* 10010 */  8,
      /* 10011 */ -1,
      /* 10100 */  5,
      /* 10101 */ -1,
      /* 10110 */ -1,
      /* 10111 */ -1,
      /* 11000 */  3,
      /* 11001 */ -1,
      /* 11010 */ -1,
      /* 11011 */ -1,
      /* 11100 */ -1,
      /* 11101 */ -1,
      /* 11110 */ -1,
      /* 11111 */ -1,
    };

    template<class Tag> 
    const int GenericItfEncoding<Tag>::invertedDecoding[32] = {
      /* 00000 */ -1,
      /* 00001 */ -1,
      /* 00010 */ -1,
      /* 00011 */ -1,
      /* 00100 */ -1,
      /* 00101 */ -1,
      /* 00110 */ -1,
      /* 00111 */  3,
      /* 01000 */ -1,
      /* 01001 */ -1,
      /* 01010 */ -1,
      /* 01011 */  5,
      /* 01100 */ -1,
      /* 01101 */  8,
      /* 01110 */  1,
      /* 01111 */ -1,
      /* 10000 */ -1,
      /* 10001 */ -1,
      /* 10010 */ -1,
      /* 10011 */  6,
      /* 10100 */ -1,
      /* 10101 */  9,
      /* 10110 */  2,
      /* 10111 */ -1,
      /* 11000 */ -1,
      /* 11001 */  0,
      /* 11010 */  4,
      /* 11011 */ -1,
      /* 11100 */  7,
      /* 11101 */ -1,
      /* 11110 */ -1,
      /* 11111 */ -1,
    };

    template<class Tag> 
    const int GenericItfEncoding<Tag>::head[2][2] = {{0, 0}, {0, 0}};

    template<class Tag> 
    const int GenericItfEncoding<Tag>::tail[2][2] = {{1, 0}, {0, 0}};

    template<class Tag> 
    const int GenericItfEncoding<Tag>::flatHead[4] = {0, 0, 0, 0};

    template<class Tag> 
    const int GenericItfEncoding<Tag>::invertedFlatHead[4] = {1, 1, 1, 1};

    template<class Tag> 
    const int GenericItfEncoding<Tag>::flatTail[3] = {1, 0, 0};

    template<class Tag> 
    const int GenericItfEncoding<Tag>::invertedFlatTail[3] = {0, 1, 1};

  } // namespace detail

  typedef detail::GenericItfEncoding<void> ItfEncoding;

} // namespace barcode


#endif // BARCODE_ITF_ENCODING_H
