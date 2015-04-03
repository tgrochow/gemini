#include <functional>
#include <sstream>

#include <sys/stat.h>

#include <rule_set.hpp>


namespace gemini
{

rule_set::rule_set(std::string const& path) :
path_(path)
{}


bool rule_set::permission(descriptor const& desc)
{
  unsigned short evaluation = IGNORE;

  for(auto rule_it = rules_.begin() ; rule_it != rules_.end() ; ++rule_it)
  {
    evaluation = rule_it->evaluate(desc);

    if(evaluation != IGNORE) return evaluation == PERMIT;
  }

  return evaluation;
}


void rule_set::push_back(rule const& r)
{
  rules_.push_back(r);
}

void rule_set::push_front(rule const& r)
{
  rules_.push_front(r);
}

void rule_set::clear()
{
  rules_.clear();
}


// save rule set on hard disk
void rule_set::save() const
{
  struct stat dir_status;

  // if root dir doesn't exist (UNIX)
  if(stat(ROOT_DIR_,&dir_status) != 0 || !S_ISDIR(dir_status.st_mode))
  {
    // make root directory (UNIX)
    mkdir(ROOT_DIR_,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
    

  // open output file stream, overwrite old file
  std::ofstream out(ROOT_DIR_ + path_ ,
                    std::ofstream::out | std::ofstream::trunc);

  // output file stream is valid (no errors, correct permissions)
  if(out.good())
  {
    // for every rule
    for(auto rule_it = rules_.begin() ; rule_it != rules_.end() ; ++rule_it)
    {
      // stream current rule
      out << *(rule_it);
    }

    // close file stream
    out.close();
  }
}


// load rule set from hard disk
void rule_set::load(std::string const& path)
{
  // open input filestream
  std::ifstream in(ROOT_DIR_ + path , std::ifstream::in);

  // input file stream is open (exists, not damaged,correct permissions)
  if(in.is_open())
  {
    // set new path
    path_ = path;

    // clear the current rule set
    rules_.clear();


    // input line
    std::string line;

    // rule description
    std::array<unsigned short,DESCRIPTOR_SIZE> info;

    // rule permission
    bool permission;

    // function checks if character must be removed before parsing the rule
    std::function<bool(char)>valid_rule_info([](char c)
    {
      // character is not a digit nor a space
      return !isdigit(c) && c != ' ';
    });
   

    // for every line in file
    while(getline(in,line))
    {
      // erase every character except digits and spaces
      line.erase(std::remove_if(line.begin(),line.end(),valid_rule_info),
                 line.end()                                              );

      // create string stream from the current line
      std::stringstream rule_string(line);

      // for every information type
      for(unsigned short index = BUS ; index != UNDEFINED ; ++index)
      {
        // stream from string to buffer
        rule_string >> info[index];
      }

      // get rule permission
      rule_string >> permission;


      // memorize current rule
      rules_.push_back(rule(descriptor(info),permission));
    }

    // close input file stream
    in.close();
  }
}


// write rule information in data stream
QDataStream & operator << (QDataStream & out_stream,rule_set const& rule_set)
{
  // for every rule
  for(auto rule_it  = rule_set.rules_.begin() ;
           rule_it != rule_set.rules_.end()   ; ++rule_it)
  {
    // write rule unreadable (better parsing)
    out_stream << rule_it->info(false).c_str();
  }

  return out_stream;
}


} // end namespace gemini
