#ifndef GEMINI_RULE_INFO
#define GEMINI_RULE_INFO

// std
#include <array>

// gemini
#include <device_info.hpp>


enum rule_value{RBUS,RPORT,RVENDOR,RDEVICE,RCLASS,RPERMISSION,RUNDEFINED};
enum rule_masked{RMASKED};

namespace gemini
{

struct rule_info
{
  rule_info();
  rule_info(std::string const& input);
  rule_info(device_info const& device_info);

  std::string const rule_string() const;


  std::array<unsigned short,RPERMISSION> values_;

  bool permission_;
};

}

#endif // GEMINI_RULE_INFO
