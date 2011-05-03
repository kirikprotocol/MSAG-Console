#ifndef __SMSC_UTIL_CONFIG_REGION_REGIONFINDER_HPP__
# define __SMSC_UTIL_CONFIG_REGION_REGIONFINDER_HPP__ 1

# include <string>
# include <map>
# include <util/config/region/Region.hpp>
# include <core/buffers/XTree.hpp>
# include <util/Singleton.hpp>

namespace smsc {
namespace util {
namespace config {
namespace region {

class RegionFinder : public smsc::util::Singleton<RegionFinder> {
public:
  RegionFinder():_regionDefault(0)
  {
  }
  void unsafeReset(); // destroy the mapping
  void registerAddressMask(const std::string& addressMask, const smsc::util::config::region::Region* region);
  void registerDefaultRegion(const smsc::util::config::region::RegionDefault* region);
  const smsc::util::config::region::Region* findRegionByAddress(const std::string& address);
  const smsc::util::config::region::Region* getRegionById(const std::string& regionId);
    const smsc::util::config::region::RegionDefault* getDefaultRegion() { return _regionDefault; }
private:
  smsc::core::buffers::XTree<const smsc::util::config::region::Region*,smsc::core::buffers::HeapAllocator,false> _wildcardSearchTree;
  const smsc::util::config::region::RegionDefault* _regionDefault;
  typedef std::map<std::string, const smsc::util::config::region::Region*> regions_hash_t;
  regions_hash_t _regionsHash;
};

}}}}

#endif
