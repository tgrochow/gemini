#include <sstream>

#include <rule_info.hpp>

namespace gemini
{

rule_info::rule_info() :
permission_(true)
{
  values_.fill(0);
}

rule_info::rule_info(std::string const& input)
{
  std::string token;
  std::istringstream ss(input);

  // extract device description
  for(unsigned short token_index  = 0 ;
                     token_index != RPERMISSION ; ++token_index)
  {
    ss >> token;

    values_[token_index] = std::stoi(token);
  }

  // extract permission
  ss >> token;

  permission_ = static_cast<bool> (std::stoi(token));
}

rule_info::rule_info(device_info const& device_info)
{
  for(unsigned short value_index = RBUS    ;
                     value_index != RCLASS ; ++value_index)
  {
    values_[value_index] = device_info.device_values_[value_index];
  }

  if(!device_info.intf_settings_.empty())
  {
    values_[RCLASS] = device_info.intf_settings_.begin()->first.front();
  }

  permission_ = device_info.intf_settings_.begin()->second;
}


std::string const rule_info::rule_string() const
{
  std::string rule_string;

  for(unsigned short token_index  = 0 ;
                     token_index != RPERMISSION ; ++token_index)
  {
    rule_string += std::to_string(values_[token_index]);

    rule_string += " ";
  }

  rule_string += std::to_string(permission_);

  return rule_string;
}

}
