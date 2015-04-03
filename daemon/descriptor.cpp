#include <algorithm>

#include <descriptor.hpp>


namespace gemini
{

descriptor::

descriptor(unsigned short bus,
           unsigned short port,
           unsigned short product_id,
           unsigned short vendor_id,
           unsigned short interface_class) :

info_(std::array<unsigned short,DESCRIPTOR_SIZE>
      {bus,port,vendor_id,product_id,interface_class})
{}

descriptor::

descriptor(std::array<unsigned short,DESCRIPTOR_SIZE> const& info) :

info_(info)
{}


bool descriptor::relevant(descriptor const& descriptor) const
{
  bool relevant = true;

  for(unsigned short index = BUS ; index != UNDEFINED ; ++index)
  {
    if(info_[index] != MASKED && info_[index] != descriptor[index])
    {
      relevant = false;
    }
  }

  return relevant;
}


void descriptor::

read_device_address(libusb_device * device)
{
  info_[BUS]    = libusb_get_bus_number(device);
  info_[PORT]   = libusb_get_port_number(device);
}

void descriptor::

read_device_descriptor(libusb_device_descriptor const& dev_desc)
{
  info_[VENDOR_ID]  = dev_desc.idVendor;
  info_[PRODUCT_ID] = dev_desc.idProduct;
}

void descriptor::

read_interface_descriptor(libusb_interface_descriptor const& intf_desc)
{
  info_[INTERFACE_CLASS] = intf_desc.bInterfaceClass;
}


std::string const descriptor::info(bool readable) const
{
    std::string descriptor_info;

    for(unsigned short index = BUS ; index != UNDEFINED ; ++index)
    {
      if(index > 0) descriptor_info += " ";

      if(readable)
      {
        switch(index)
        {
          case BUS             : descriptor_info += "[BUS:";            break;
          case PORT            : descriptor_info += "[PORT:";            break;
          case VENDOR_ID       : descriptor_info += "[VENDOR ID:";       break;
          case PRODUCT_ID      : descriptor_info += "[PRODUCT ID:";      break;
          case INTERFACE_CLASS : descriptor_info += "[INTERFACE CLASS:"; break;
        }
      }

      descriptor_info += std::to_string(info_[index])
                      +  "]";
    }

    return descriptor_info;
}

std::string const descriptor::device_info() const
{
  std::string device_info;

  for(unsigned short index = BUS ; index != INTERFACE_CLASS ; ++index)
  {
    device_info += " ";
    device_info += std::to_string(info_[index]);
  }

  return device_info;
}


unsigned short descriptor::operator [] (unsigned short index) const
{
  return info_[index];
}

unsigned short & descriptor::operator [] (unsigned short index)
{
  return info_[index];
}

bool operator == (descriptor const& i1,descriptor const& i2)
{
  bool equal =  true;

  for(unsigned short index = BUS ; index != UNDEFINED ; ++index)
  {
    if(i1[index] != i2[index]) equal = false;
  }

  return equal;
}

// stream operator
std::ostream  & operator << (std::ostream & out,descriptor const& intf_info)
{
  // write descriptor informations readable in stream
  out << intf_info.info(true);

  return out;
}

} // end namespace gemini
