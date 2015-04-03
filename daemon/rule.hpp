#ifndef GEMINI_RULE
#define GEMINI_RULE


#include <descriptor.hpp>

namespace gemini
{

enum EVALUATION{PERMIT,PROHIBIT,IGNORE};

class rule
{
  public :

  rule(descriptor const& desc,bool permission);
  rule(std::string const& input);

  unsigned short evaluate(descriptor const& intf_desc) const;

  std::string const info(bool readable) const;


  private :

  descriptor descriptor_;
  
  bool       permission_;
};

std::ostream & operator << (std::ostream & out,rule const& r);

}

#endif
