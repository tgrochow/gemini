#ifndef GEMINI_DEVICE_INFO
#define GEMINI_DEVICE_INFO

// container
#include <array>
#include <string>
#include <vector>

#include <libusb-1.0/libusb.h>


enum device_string{PRODUCT_STRING,VENDOR_STRING,UNDEFINED_STRING};
enum device_value {BUS,PORT,VENDOR_ID,PRODUCT_ID,INTERFACE_NUMBER,UNDEFINED};


#define index_value_shift UNDEFINED_STRING


namespace gemini
{

  struct device_info
  {
    // constructor
    device_info();
    device_info(std::string const& input);


    // object method
    void parse_device(std::string const& input);

    // class method
    static std::string const class_string(unsigned short class_id);


    // member
    std::array<std::string,UNDEFINED_STRING>                  device_strings_;
    std::array<unsigned short,UNDEFINED>                      device_values_;
    std::vector<std::pair<std::vector<unsigned short>,bool> > intf_settings_;
  };

  // operator
  bool operator == (device_info const& d1 , device_info const& d2);
  bool operator  < (device_info const& d1 , device_info const& d2);

}

#endif // GEMINI_DEVICE_INFO
