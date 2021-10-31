#ifndef BARCODE_ITF_BAR_H
#define BARCODE_ITF_BAR_H

namespace barcode {
// -------------------------------------------------------------------------- //
// ItfBar
// -------------------------------------------------------------------------- //
  /**
   * Single bar of an Interleaved 2 of 5 barcode.
   */
  class ItfBar {
  public:
    enum Thickness {
      THIN,
      THICK
    };

    enum Color {
      BLACK,
      WHITE
    };

    ItfBar(Thickness thickness, Color color): mIsThick(thickness != THIN), mIsWhite(color != BLACK) {}

    ItfBar(bool isThick, bool isWhite): mIsThick(isThick), mIsWhite(isWhite) {}

    bool isThick() const {
      return mIsThick;
    }

    bool isThin() const {
      return !mIsThick;
    }

    bool isWhite() const {
      return mIsWhite;
    }

    bool isBlack() const {
      return !mIsWhite;
    }

    Thickness thickness() const {
      return mIsThick ? THICK : THIN;
    }

    Color color() const {
      return mIsWhite ? WHITE : BLACK;
    }

  private:
    bool mIsThick;
    bool mIsWhite;
  };

} // namespace barcode

#endif // BARCODE_ITF_BAR_H
