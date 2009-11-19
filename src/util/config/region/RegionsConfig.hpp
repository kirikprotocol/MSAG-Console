#ifndef __SMSC_UTIL_CONFIG_REGION_REGIONSCONFIG_HPP__
# define __SMSC_UTIL_CONFIG_REGION_REGIONSCONFIG_HPP__ 1

# include <string>
# include <map>
# include <util/config/region/Region.hpp>

# include <xercesc/dom/DOM.hpp>

namespace smsc {
namespace util {
namespace config {
namespace region {

class RegionsConfig
{
  typedef std::map<std::string, Region*> Regions_t;
public:
  RegionsConfig(const std::string& filename) : _filename(filename) {}
  ~RegionsConfig();
  enum status {success, fail};

  status load();
  status reload() { return fail; }

  class RegionsIterator;
  RegionsIterator getIterator();

  class RegionsIterator
  {
  public:
    bool hasRecord() {return _iter != _end;}
    status fetchNext(Region *&record);
  private:
    RegionsIterator(const Regions_t::iterator& begin, const Regions_t::iterator& end)
      : _iter(begin), _end(end) {}
    friend class RegionsConfig;
    Regions_t::iterator _iter;
    Regions_t::iterator _end;
  };

  const Region& getRegion(const std::string& regionId) const;
  const RegionDefault& getDefaultRegion() const;
private:
  status loadSubjects(xercesc::DOMNodeList *subjects_elements, Region::subjects_ids_lst_t& subjects_ids);

  std::string _filename;

  Regions_t _regions;
  Region _nullRegion;
  RegionDefault _defaultRegion;
};

}}}}

#endif
