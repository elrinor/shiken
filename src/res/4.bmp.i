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
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 143,  64,  64, 112, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 223,   0,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255,  96,   0,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 223,   0,   0,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255,  96,   0,   0,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 223,   0,   0,   0,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255,  96,   0,   0,  32,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 223,   0,   0,  96,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255,  96,   0,   0, 223,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 191,   0,   0,  96, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255,  64,   0,   0, 223, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 191,   0,   0,  80, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255,  64,   0,   0, 191, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 191,   0,   0,  64, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255,  64,   0,   0, 191, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 191,   0,   0,  64, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255,  64,   0,   0, 191, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 159,   0,   0,  64, 255, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255,  32,   0,   0,  80, 128, 128, 128, 128, 128,  32,   0,   0,  32, 128, 128, 159, 255, 255, 255, 
      255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  64, 255, 255, 255, 
      255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  64, 255, 255, 255, 
      255, 191, 191, 191, 191, 191, 191, 191, 191, 191,  48,   0,   0,  48, 191, 191, 207, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  64,   0,   0,  64, 255, 255, 255, 255, 255, 255, 
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 112,  64,  64, 112, 255, 255, 255, 255, 255, 255, 
        0
    };
  } // namespace detail
  
  typedef detail::GenericImage<void> Image;
} // namespace gencimg 

