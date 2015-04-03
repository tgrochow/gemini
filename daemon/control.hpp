#ifndef GEMINI_CONTROL
#define GEMINI_CONTROL


#include <list>
#include <vector>

#include <descriptor.hpp>
#include <device_list.hpp>
#include <rule_set.hpp>


namespace gemini
{

class control
{
  public :

  control();

  void enforce_rule_set(bool gather_intf_info = false);

  // get interface info for client applications
  std::vector<std::string> const interface_info() const;


  rule_set rule_set_;


  private :

  void disable(libusb_device * device,int interface_id,descriptor const& desc);

  void enable(libusb_device * device,int interface_id,
              std::list<descriptor>::iterator desc_it);

  std::string const read_string_descriptor(libusb_device * device_handle,
                                           uint8_t         index         );


  device_list device_list_;

  std::list<descriptor>    disabled_;
  std::vector<std::string> intf_info_;
};

}

#endif
