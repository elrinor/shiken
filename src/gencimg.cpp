#include "config.h"
#include <iostream>
#include <exception>
#include <iomanip>
#include <vigra/stdimage.hxx>
#include "ImageUtils.h"
#include "Common.h"

int main(int argc, char** argv) {
  using namespace std;

  if(argc != 2) {
    cout << "gencimg - generator of C code representation for grayscale images, version " << BRT_VERSION << "." << endl;
    cout << endl;
    cout << "USAGE:" << endl;
    cout << "  gencbmp <input.image>" << endl;
    return 1;   
  }

  try {
    vigra::BImage image;
    importImage(image, string(argv[1]));

    cout << "namespace gencimg {" << endl;
    cout << "  namespace detail {" << endl;
    cout << "    template<class Tag>" << endl;
    cout << "    class GenericImage {" << endl;
    cout << "    public:" << endl;
    cout << "      static const vigra::BImage& image() {" << endl;
    cout << "        return bImage;" << endl;
    cout << "      }" << endl;
    cout << "      " << endl;
    cout << "    private:" << endl;
    cout << "      static const int width = " << image.width() << ";" << endl;
    cout << "      static const int height = " << image.height() << ";" << endl;
    cout << "      static const vigra::UInt8 data[" << image.width() * image.height() + 1 << "];" << endl;
    cout << "      static const vigra::BImage bImage;" << endl;
    cout << "    };" << endl;
    cout << "    " << endl;
    cout << "    template<class Tag> const vigra::BImage GenericImage<Tag>::bImage(width, height, data);" << endl;
    cout << "    template<class Tag> const vigra::UInt8 GenericImage<Tag>::data[" << image.width() * image.height() + 1 << "] = {" << endl;
    for(int y = 0; y < image.height(); y++) {
      cout << "      ";
      for(int x = 0; x < image.width(); x++)
        cout << setw(3) << setfill(' ') << static_cast<int>(image[vigra::Diff2D(x,y)]) << ", ";
      cout << endl;
    }
    cout << "        0" << endl;
    cout << "    };" << endl;
    cout << "  } // namespace detail" << endl;
    cout << "  " << endl;
    cout << "  typedef detail::GenericImage<void> Image;" << endl;
    cout << "} // namespace gencimg " << endl;
    cout << endl;
  } catch (exception& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  } catch(...) {
    cerr << "error: Exception of unknown type." << endl;
    return 1;
  }

  return 0;
}