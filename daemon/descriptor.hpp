#ifndef GEMINI_DESCRIPTOR
#define GEMINI_DESCRIPTOR


#include <array>
#include <fstream>

#include <QtNetwork>

#include <libusb-1.0/libusb.h>


enum information_type{BUS,PORT,VENDOR_ID,PRODUCT_ID,INTERFACE_CLASS,UNDEFINED};
enum mask{MASKED};

#define DESCRIPTOR_SIZE UNDEFINED

namespace gemini
{

class descriptor
{
  public :

  descriptor(unsigned short bus             = MASKED ,
             unsigned short port            = MASKED ,
             unsigned short vendor_id       = MASKED ,
             unsigned short product_id      = MASKED ,
             unsigned short interface_class = MASKED  );

  descriptor(std::array<unsigned short,DESCRIPTOR_SIZE> const& info);

  unsigned short   operator [] (unsigned short index) const;
  unsigned short & operator [] (unsigned short index);

  bool relevant(descriptor const& desc) const;

  void read_device_address(libusb_device * device);
  void read_device_descriptor(libusb_device_descriptor const& dev_desc);
  void read_interface_descriptor(libusb_interface_descriptor const& intf_desc);

  std::string const info(bool readable) const;
  std::string const device_info() const;


  private :

  std::array<unsigned short,DESCRIPTOR_SIZE> info_;
};

bool operator == (descriptor const& d1,descriptor const& d2);

std::ostream  & operator << (std::ostream & out,descriptor const& descriptor);

}


#endif // GEMINI_DESCRIPTOR
