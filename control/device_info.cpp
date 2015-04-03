#include <algorithm>
#include <sstream>

#include <device_info.hpp>

namespace gemini
{

device_info::device_info()
{
  device_strings_.fill("undefined");
  device_values_.fill(0);
}

device_info::device_info(std::string const& input)
{
  parse_device(input);
}


void device_info::parse_device(std::string const& input)
{
    std::string token;
    std::istringstream ss(input);

    // extract device description
    for(unsigned short string_index  = PRODUCT_STRING ;
                       string_index != UNDEFINED_STRING ; ++string_index)
    {
      ss >> device_strings_[string_index];


      std::replace(device_strings_[string_index].begin(),
                   device_strings_[string_index].end()  ,'_',' ');
    }


    // extract device definition
    for(unsigned short value_index  = BUS ;
                       value_index != UNDEFINED ; ++value_index)
    {
      ss >> token;

      device_values_[value_index] = std::stoi(token);
    }


    // extract interface definition
    for(unsigned short intf = 0 ;
                       intf < device_values_[INTERFACE_NUMBER] ; ++intf)
    {
      ss >> token;


      std::vector<unsigned short> settings;

      unsigned short setting_number = std::stoi(token);

      settings.reserve(setting_number);


      for(unsigned short setting_index = 0 ;
                         setting_index < setting_number ; ++setting_index)
      {
        ss >> token;

        settings.push_back(std::stoi(token));
      }


      // extract interface permission
      ss >> token;

      bool permission = static_cast<bool> (std::stoi(token));

      intf_settings_.push_back(std::make_pair(settings,permission));
    }
}


bool operator < (device_info const& d1 , device_info const& d2)
{
  for(unsigned short value_index(BUS) ;
                     value_index < INTERFACE_NUMBER ; ++value_index)
  {
    if(d1.device_values_[value_index] != d2.device_values_[value_index])
    {
      return d1.device_values_[value_index] < d2.device_values_[value_index];
    }
  }

  return false;
}


bool operator == (device_info const& d1 , device_info const& d2)
{
  return !(d1 < d2) && !(d2 < d1);
}


std::string const device_info::class_string(unsigned short class_id)
{
  std::string interface_class("");

  switch(class_id)
  {
    case  LIBUSB_CLASS_PER_INTERFACE :

          interface_class = "INTERFACE SPEC";
          break;

    case  LIBUSB_CLASS_AUDIO :

          interface_class = "AUDIO";
          break;

    case  LIBUSB_CLASS_COMM :

          interface_class = "COMMUNICATION";
          break;

    case  LIBUSB_CLASS_HID :

          interface_class = "HID";
          break;

    case  LIBUSB_CLASS_PHYSICAL :

          interface_class = "PHYSICAL";
          break;

    case  LIBUSB_CLASS_PRINTER  :

          interface_class = "PRINTER";
          break;

    case  LIBUSB_CLASS_PTP :

          interface_class = "PTP";
          break;

    case  LIBUSB_CLASS_MASS_STORAGE :

          interface_class = "MASS STORAGE";
          break;

    case  LIBUSB_CLASS_HUB :

          interface_class = "HUB";
          break;

    case LIBUSB_CLASS_DATA :

         interface_class = "DATA";
         break;

    case LIBUSB_CLASS_SMART_CARD :

         interface_class = "SMART CARD";
         break;

    case LIBUSB_CLASS_CONTENT_SECURITY :

         interface_class = "CONTENT SECURITY";
         break;

    case LIBUSB_CLASS_VIDEO :

         interface_class = "VIDEO";
         break;

    case LIBUSB_CLASS_PERSONAL_HEALTHCARE :

         interface_class = "PERSONAL HEALTHCARE" ;
         break;

    case LIBUSB_CLASS_DIAGNOSTIC_DEVICE :

         interface_class = "DIAGNOSTIC DEVICE";
         break;

    case LIBUSB_CLASS_WIRELESS :

         interface_class = "WIRELESS";
         break;

    case LIBUSB_CLASS_APPLICATION :

         interface_class = "APPLICATION";
         break;

    case LIBUSB_CLASS_VENDOR_SPEC :

         interface_class = "VENDOR SPEC";
         break;

    default :

      interface_class = "UNDEFINED";
      break;
  }


  return interface_class;
}

} // namespace gemini end
