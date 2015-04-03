#ifndef GEMINI_RULE_SET
#define GEMINI_RULE_SET


#include <list>
#include <string>

#include <rule.hpp>

namespace gemini
{

class rule_set
{
  public :

  rule_set(std::string const& path = "default.rules");

  bool permission(descriptor const& desc);

  void push_back(rule const& r);
  void push_front(rule const& r);
  void clear();

  void save() const;
  void load(std::string const& path = "default.rules");

  friend QDataStream & operator << (QDataStream & out_stream,
                                    rule_set const& rule_set);


  private :

  std::list<rule> rules_;

  std::string path_;

  const char * ROOT_DIR_ = "/etc/usb_filter/";
};

}

#endif // GEMINI_RULE_SET
