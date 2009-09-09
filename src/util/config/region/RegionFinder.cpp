#include "RegionFinder.hpp"
#include <stdio.h>

namespace smsc {
namespace util {
namespace config {
namespace region {

void RegionFinder::unsafeReset()
{
    _wildcardSearchTree.Clear();
    _regionDefault = 0;
    _regionsHash.clear();
}

void
RegionFinder::registerAddressMask(const std::string& addressMask, const smsc::util::config::region::Region* region)
{
  _wildcardSearchTree.Insert(addressMask.c_str(), region);
  regions_hash_t::const_iterator iter = _regionsHash.find(region->getId());
  if ( iter == _regionsHash.end() )
   _regionsHash.insert(std::make_pair(region->getId(), region));
}

void
RegionFinder::registerDefaultRegion(const smsc::util::config::region::RegionDefault* region)
{
  _regionDefault = region;
  _regionsHash[region->getId()] = region;
}

const smsc::util::config::region::Region*
RegionFinder::findRegionByAddress(const std::string& address)
{
  const smsc::util::config::region::Region* regionPtr = NULL;
  bool ret = _wildcardSearchTree.Find(address.c_str(), regionPtr);
  if ( !ret ) return _regionDefault;
  else return regionPtr;
}

const smsc::util::config::region::Region*
RegionFinder::getRegionById(const std::string& regionId)
{
  regions_hash_t::const_iterator iter = _regionsHash.find(regionId);
  if ( iter != _regionsHash.end() ) return iter->second;
  else return NULL;
}

}}}}
