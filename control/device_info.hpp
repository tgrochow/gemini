#ifndef GEMINI_DEVICE_INFO
#define GEMINI_DEVICE_INFO

// std
#include <array>
#include <string>
#include <vector>

// libusb
#include <libusb-1.0/libusb.h>


namespace gemini
{
  enum device_string{PRODUCT_STRING,VENDOR_STRING,UNDEFINED_STRING};
  enum device_value {BUS,PORT,VENDOR_ID,PRODUCT_ID,INTERFACE_NUMBER,UNDEFINED};

  // shift
  #define index_value_shift UNDEFINED_STRING

  struct device_info
  {
    // constructor
    device_info();
    device_info(std::string const& input);


    // parse request string, create device information
    void parse_device(std::string const& input);

    // readable usb class string
    static std::string const class_string(unsigned short class_id);


    // product, vendor string
    std::array<std::string,UNDEFINED_STRING>                  device_strings_;
    // bus, port, vendor id, product id, interface number
    std::array<unsigned short,UNDEFINED>                      device_values_;
    // setting class, permission of every interface
    std::vector<std::pair<std::vector<unsigned short>,bool> > intf_settings_;
  };

  // operators
  bool operator  < (device_info const& d1 , device_info const& d2);
  bool operator == (device_info const& d1 , device_info const& d2);
}

#endif // GEMINI_DEVICE_INFO
