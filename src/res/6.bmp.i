namespace gencimg {
  namespace detail {
    template<class Tag>
    class GenericImage {
    public:
      static const vigra::BImage& image() {
        return bImage;
      }
      
    private:
      static const int width = 20;
      static const int height = 28;
      static const vigra::UInt8 data[561];
      static const vigra::BImage bImage;
    };
    
    template<class Tag> const vigra::BImage GenericImage<Tag>::bImage(width, height, data);
    template<class Tag> const vigra::UInt8 GenericImage<Tag>::data[561] = {
      255, 255, 255, 255, 255, 255, 255, 223, 143,  80,  64, 112, 191, 255, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 159,  16,   0,   0,   0,   0,   0,  96, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 159,   0,   0,   0,   0,   0,   0,   0,   0, 143, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 191,   0,   0,   0, 128, 239, 255,  96,   0,   0,  16, 239, 255, 255, 255, 255, 
      255, 255, 255, 255,  32,   0,   0, 112, 255, 255, 255, 255,  32,   0,   0, 159, 255, 255, 255, 255, 
      255, 255, 255, 159,   0,   0,  16, 239, 255, 255, 255, 255, 112,   0,   0,  96, 255, 255, 255, 255, 
      255, 255, 255,  48,   0,   0, 112, 255, 255, 255, 255, 255, 159,  64,  64, 112, 255, 255, 255, 255, 
      255, 255, 223,   0,   0,   0, 207, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
      255, 255, 159,   0,   0,  16, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
      255, 255,  96,   0,   0,  80, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
      255, 255,  64,   0,   0, 128, 255, 191,  80,   0,   0,  64, 175, 255, 255, 255, 255, 255, 255, 255, 
      255, 255,  16,   0,   0, 159, 159,   0,   0,   0,   0,   0,   0,  96, 255, 255, 255, 255, 255, 255, 
      255, 255,   0,   0,   0, 112,   0,   0,  80, 128,  64,   0,   0,   0, 128, 255, 255, 255, 255, 255, 
      255, 255,   0,   0,   0,   0,  16, 191, 255, 255, 255, 112,   0,   0,  16, 239, 255, 255, 255, 255, 
      255, 255,   0,   0,   0,   0, 143, 255, 255, 255, 255, 255,  32,   0,   0, 143, 255, 255, 255, 255, 
      255, 255,   0,   0,   0,  32, 255, 255, 255, 255, 255, 255, 112,   0,   0,  64, 255, 255, 255, 255, 
      255, 255,   0,   0,   0,  96, 255, 255, 255, 255, 255, 255, 175,   0,   0,  16, 255, 255, 255, 255, 
      255, 255,   0,   0,   0, 128, 255, 255, 255, 255, 255, 255, 191,   0,   0,   0, 255, 255, 255, 255, 
      255, 255,  64,   0,   0, 128, 255, 255, 255, 255, 255, 255, 191,   0,   0,   0, 255, 255, 255, 255, 
      255, 255,  80,   0,   0, 128, 255, 255, 255, 255, 255, 255, 191,   0,   0,   0, 255, 255, 255, 255, 
      255, 255, 128,   0,   0,  80, 255, 255, 255, 255, 255, 255, 191,   0,   0,   0, 255, 255, 255, 255, 
      255, 255, 191,   0,   0,  32, 255, 255, 255, 255, 255, 255, 143,   0,   0,  64, 255, 255, 255, 255, 
      255, 255, 255,  16,   0,   0, 191, 255, 255, 255, 255, 255,  96,   0,   0, 112, 255, 255, 255, 255, 
      255, 255, 255,  96,   0,   0,  64, 255, 255, 255, 255, 239,  16,   0,   0, 191, 255, 255, 255, 255, 
      255, 255, 255, 223,  16,   0,   0,  96, 239, 255, 239,  48,   0,   0,  64, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 175,   0,   0,   0,  16,  64,   0,   0,   0,  16, 207, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 175,  16,   0,   0,   0,   0,   0,  16, 207, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 239, 143,  64,  64,  80, 143, 239, 255, 255, 255, 255, 255, 255, 255, 
        0
    };
  } // namespace detail
  
  typedef detail::GenericImage<void> Image;
} // namespace gencimg 
