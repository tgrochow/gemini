#include <algorithm>
#include <iostream>
#include <control.hpp>

namespace gemini
{

control::control()
{
  device_list_.init();
}


void control::enforce_rule_set(bool gather_intf_info)
{
  // library was correct initialized
  if(device_list_.init_error() == LIBUSB_SUCCESS)
  {
    // libusb native typs
    libusb_device                 * device;
    libusb_config_descriptor      * config_descriptor;
    libusb_device_descriptor        device_descriptor;
    libusb_interface                interface;
    libusb_interface_descriptor     interface_descriptor;

    // libusb errors
    int device_descriptor_error = 0,
        config_descriptor_error = 0;

    // gemini native types
    descriptor                      rule_desc;
    std::list<descriptor>::iterator descriptor_iterator;

    // device information strings
    std::string                     intf_info(""),
                                    product_string("undefined"),
                                    vendor_string("undefined");

    bool                            intf_permission;



    // if method should gather device info strings, clear the old
    if(gather_intf_info) intf_info_.clear();


    // scan new device list
    ssize_t device_number = device_list_.scan();

    // for every usb device
    for(ssize_t device_id = 0 ; device_id < device_number ; ++device_id)
    {
      // get actual device
      device = device_list_.get_device();


      // try to read device descriptor
      device_descriptor_error =

      libusb_get_device_descriptor(device,&device_descriptor);

      // try to read config descriptor
      config_descriptor_error = 

      libusb_get_active_config_descriptor(device,&config_descriptor);

      // device descriptor couldn't be read
      if(device_descriptor_error != LIBUSB_SUCCESS)
      {
        // config descriptor has allocated memory
        if(config_descriptor_error == LIBUSB_SUCCESS)
        {
          // important frees allocated memory from config descriptor
          libusb_free_config_descriptor(config_descriptor);
        }

        continue;
      }

      // config descriptor couldn't be read
      if(config_descriptor_error != LIBUSB_SUCCESS)
      {
        continue;
      }

      // gather device information
      rule_desc.read_device_address(device);
      rule_desc.read_device_descriptor(device_descriptor);

      // gather device information
      if(gather_intf_info)
      {
        product_string =

        read_string_descriptor(device,device_descriptor.iProduct);

        vendor_string  =

        read_string_descriptor(device,device_descriptor.iManufacturer);

        std::replace(product_string.begin(),product_string.end(),' ','_');
        std::replace(vendor_string.begin(),vendor_string.end(),' ','_');

        // append device description
        intf_info = product_string
                  + " "
                  + vendor_string
                  + rule_desc.device_info()
                  + " "
                  + std::to_string(config_descriptor->bNumInterfaces);
      }


      // for every interface on specific device config
      for(uint8_t intf = 0 ; intf < config_descriptor->bNumInterfaces; ++intf)
      {
        // reset interface permission
        intf_permission = true;

        // get actual interface
        interface = config_descriptor->interface[intf];


        if(gather_intf_info)
        {
          intf_info += " " + std::to_string(interface.num_altsetting);
        }


        // for every setting on interface
        for(int setting = 0 ; setting < interface.num_altsetting ; ++setting)
        {
          // get interface descriptor for setting
          interface_descriptor = interface.altsetting[setting];


          // gather interface information
          rule_desc.read_interface_descriptor(interface_descriptor);

          // append setting interface class
          if(gather_intf_info)
          {
            intf_info += " "
                      +  std::to_string(interface_descriptor.bInterfaceClass);
          }


          // actual interface is prohibited
          if(intf_permission && rule_set_.permission(rule_desc) == false)
          {
            // remove kernel driver
            disable(device,intf,rule_desc);

            intf_permission = false;
          }

          // actual interface is permitted
          else if(intf_permission)
          {
            // find interface in disabled list
            descriptor_iterator = std::find(disabled_.begin(),
                                            disabled_.end()  ,
                                            rule_desc         );

            // interface is in disabled list
            if(descriptor_iterator != disabled_.end())
            {
              // reattach kernel driver
              enable(device,intf,descriptor_iterator);
            }
          }
        }

        // append permission on interface info string
        if(intf_permission) intf_info += " 1";
        else                intf_info += " 0";
      }


      // memorize actual interface info
      if(gather_intf_info) intf_info_.push_back(intf_info);


      // important frees allocated memory from config descriptor
      libusb_free_config_descriptor(config_descriptor);   
    }
  }
}


// get complete interface information
std::vector<std::string> const control::interface_info() const
{
  return intf_info_;
}


// disable a device for usb communication
void control::disable(libusb_device * device , int interface_id ,
                      descriptor const& desc)
{
  libusb_device_handle * device_handle;

  int open_error = libusb_open(device,&device_handle);

  if(open_error == LIBUSB_SUCCESS)
  {
    int kernel_driver = libusb_kernel_driver_active(device_handle,interface_id); 
  

    if(kernel_driver == 1)
    {
      int dettach_error = 

      libusb_detach_kernel_driver(device_handle,interface_id);

      if(dettach_error == LIBUSB_SUCCESS)
      {
        disabled_.push_back(desc);
      }
    }


    libusb_close(device_handle);
  }
}

// enable a device for usb communication
void control::enable(libusb_device * device , int interface_id,
                     std::list<descriptor>::iterator desc_it)
{
  libusb_device_handle * device_handle;

  int open_error = libusb_open(device,&device_handle);

  if(open_error == LIBUSB_SUCCESS)
  {
    int kernel_driver = libusb_kernel_driver_active(device_handle,interface_id); 
  

    if(kernel_driver == 0)
    {
      int attach_error = 

      libusb_attach_kernel_driver(device_handle,interface_id);

      if(attach_error == LIBUSB_SUCCESS)
      {
        disabled_.erase(desc_it);
      }
    }


    libusb_close(device_handle);
  }
}


// read a string descriptor of a device
std::string const control::

read_string_descriptor(libusb_device * device,uint8_t index)
{
  std::string string_desc("undefined");

  libusb_device_handle * device_handle;


  int open_error = libusb_open(device,&device_handle);

  if(open_error == LIBUSB_SUCCESS)
  {
    const unsigned short max_length = 128;

    unsigned char * buffer = new unsigned char[max_length];


    int char_number =

    libusb_get_string_descriptor_ascii(device_handle,index,buffer,max_length);


    if(char_number > 0)
    {
      string_desc = reinterpret_cast<char *> (buffer);

      string_desc = string_desc.substr(0,char_number);
    }

    else
    {
      string_desc = "undefined";
    }


    delete buffer;

    libusb_close(device_handle);
  }


  return string_desc;
}

}
