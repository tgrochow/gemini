#include <device_list.hpp>


namespace gemini
{

device_list::device_list() :
init_error_(0),
device_index_(0),
device_number_(0)
{}


device_list::~device_list()
{
  if(device_number_ > 0)            libusb_free_device_list(device_list_,1);

  if(init_error_ == LIBUSB_SUCCESS) libusb_exit(lib_context_);
}


int device_list::init()
{
  init_error_ = libusb_init(&lib_context_);

  if(init_error_ == LIBUSB_SUCCESS)
  {
    libusb_set_debug(lib_context_,0);
  }


  return init_error_; 
}


int device_list::scan()
{
  if(init_error_ == LIBUSB_SUCCESS)
  {
    if(device_number_ > 0) libusb_free_device_list(device_list_,1);

    device_index_  = 0;
    device_number_ = libusb_get_device_list(lib_context_,&device_list_);
  }

  else
  {
    device_number_ = LIBUSB_ERROR_OTHER;
  }


  return device_number_;
}


int device_list::init_error() const
{
  return init_error_;
}


libusb_device * device_list::get_device()
{
  if(device_number_ <= 0 || device_index_ == device_number_)
  {
    return nullptr;
  }


  libusb_device * device = device_list_[device_index_];

  ++device_index_;


  return device;
}

}
