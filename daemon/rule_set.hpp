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

  rule_set(std::string const& path = "/etc/gemini/default.rules");

  static std::string const gemini_home_path();


  bool permission(descriptor const& desc);

  void push_back(rule const& r);
  void push_front(rule const& r);
  void clear();

  void path(std::string const& new_path);
  std::string const path() const;

  void save() const;
  void load(std::string const& path);

  friend QDataStream & operator << (QDataStream & out_stream,
                                    rule_set const& rule_set);

  private :

  std::list<rule> rules_;

  std::string path_;
};

}

#endif // GEMINI_RULE_SET
