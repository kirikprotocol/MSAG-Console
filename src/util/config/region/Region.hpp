#ifndef __SMSC_UTIL_CONFIG_REGION_REGION_HPP__
# define __SMSC_UTIL_CONFIG_REGION_REGION_HPP__ 1

# include <sys/types.h>
# include <vector>
# include <string>
# include <util/config/route/RouteConfig.h>

namespace smsc {
namespace util {
namespace config {
namespace region {

class Region
{
public:
  typedef std::vector<std::string> subjects_ids_lst_t;
  Region(const std::string& id="", const std::string& name="", ulong_t bandwidth=0, const std::string& email="",subjects_ids_lst_t subjects_ids=subjects_ids_lst_t());

  const std::string& getId() const { return _id; }
  const std::string& getName() const { return _name; }
  ulong_t getBandwidth() const { return _bandwidth; }
  std::string getEmail()const
  {
    return _email;
  }

  std::string toString() const;

  void expandSubjectRefs(smsc::util::config::route::RouteConfig& routeCfg);
private:
  std::string _id, _name;
  ulong_t _bandwidth;
  std::string _email;
  subjects_ids_lst_t _subjects_ids;

  typedef std::vector<smsc::util::config::route::Subject> subjects_t;
  subjects_t _subjects;

  bool _subjectHasBeenExpanded;

public:
  class MasksIterator
  {
  public:
    MasksIterator(Region& region);
    bool fetchNext(std::string& mask);
  protected:
    bool hasRecord();

    std::vector<std::string> _masks;
    std::vector<std::string>::iterator _iter;
    std::vector<std::string>::iterator _end;
  };

  MasksIterator getMasksIterator();
};

class RegionDefault : public Region
{
public:
  RegionDefault(ulong_t bandwidth=0,const std::string& email="")
    : Region("regDefault", "default region", bandwidth,email) {}
};
}}}}

#endif
