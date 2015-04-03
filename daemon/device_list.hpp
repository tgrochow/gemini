#ifndef GEMINI_DEVICE_LIST
#define GEMINI_DEVICE_LIST


#include <libusb-1.0/libusb.h>

#include <descriptor.hpp>


namespace gemini
{

class device_list
{
  public :

  device_list();
  ~device_list();

  int init();
  int scan();

  int init_error() const;

  libusb_device * get_device();


  private :

  int              init_error_;

  ssize_t          device_index_,
                   device_number_;

  libusb_context * lib_context_;
  libusb_device ** device_list_;
};

}


#endif
