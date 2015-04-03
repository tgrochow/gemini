#include <sstream>

#include <rule.hpp>

namespace gemini
{
  rule::rule(descriptor const& desc,bool permission) :
  descriptor_(desc),
  permission_(permission)
  {}

  rule::rule(std::string const& input)
  {
    std::string token;
    std::istringstream ss(input);

    // extract device description
    for(unsigned short token_index  = BUS ;
                       token_index != UNDEFINED ; ++token_index)
    {
      ss >> token;

      descriptor_[token_index] = std::stoi(token);
    }


    ss >> token;

    permission_ = static_cast<bool> (std::stoi(token));
  }


  unsigned short rule::evaluate(descriptor const& intf_desc) const
  {
    unsigned short warrant;         


    if(!descriptor_.relevant(intf_desc))
    {
      warrant = IGNORE;
    }

    else if(permission_)
    {
      warrant = PERMIT;
    }

    else
    {
      warrant = PROHIBIT;
    }


    return warrant;
  }


  std::string const rule::info(bool readable) const
  {
    std::string rule_info(descriptor_.info(readable));

    rule_info += " ";

    if(readable) rule_info += "[PERMISSION]";


    rule_info += std::to_string(permission_);


    return rule_info;
  }


  std::ostream & operator << (std::ostream & out,rule const& r)
  {
    out << r.info(true)

        << std::endl;


    return out;
  }

 }
