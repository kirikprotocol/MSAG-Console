package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class RegionsSettings {

  private Map<String, Region> regionsByMasks = new HashMap<String, Region>();
  private Map<String, Region> regions = new LinkedHashMap<String, Region>();

  RegionsSettings(Collection<Region> regions) {
    for(Region r : regions) {
      this.regions.put(r.getRegionId(), r);
      for(Address a : r.getMasks()) {
        regionsByMasks.put(a.getSimpleAddress(), r);
      }
    }

  }

  private synchronized String getNextId() {
    int maxId = -1;
    for (Region r : regions.values())
      maxId = Math.max(maxId, Integer.parseInt(r.getRegionId()));
    return (maxId + 1) + "";

  }

  private void checkMask(Region region) throws AdminException{
    for(Address a : region.getMasks()) {
      Region exist = regionsByMasks.get(a.getSimpleAddress());
      if(exist != null && !exist.getRegionId().equals(region.getRegionId())) {
        throw new RegionException("masks_intersection", a.getSimpleAddress());
      }
    }
  }

  void addRegion(Region region) throws AdminException{
    checkMask(region);
    region.setRegionId(getNextId());
    for(Address a : region.getMasks()) {
      regionsByMasks.put(a.getSimpleAddress(), region);
    }
    regions.put(region.getRegionId(), region);
  }

  void removeRegion(String regionId) throws AdminException {
    Region r = regions.remove(regionId);
    if(r != null) {
      for(Address a : r.getMasks()) {
        regionsByMasks.remove(a.getSimpleAddress());
      }
    }
  }

  void updateRegion(Region region) throws AdminException {
    checkMask(region);
    Region old = regions.remove(region.getRegionId());
    if(old == null) {
      throw new RegionException("region_not_exist",region.getName());
    }
    regions.put(region.getRegionId(), region);
    for(Address a : old.getMasks()) {
      regionsByMasks.remove(a.getSimpleAddress());
    }
    for(Address a : region.getMasks()) {
      regionsByMasks.put(a.getSimpleAddress(), region);
    }
  }


  Region getRegion(String id) {
    Region r = regions.get(id);
    return r == null ? null : r.cloneRegion();
  }

  Region getRegionByAddress(Address address) {
    String mask = address.getSimpleAddress();
    Region r = regionsByMasks.get(mask);
    while (r == null && !mask.startsWith("?")){
      int i = mask.indexOf("?");
      if(i>0) {
        mask = mask.substring(0, i-1)+'?'+mask.substring(i);
      }else if(i == -1) {
        mask = mask.substring(0, mask.length()-1)+'?';
      }
      r = regionsByMasks.get(mask);
    }
    return r;
  }


  List<Region> getRegions() {
    return new ArrayList<Region>(regions.values());
  }


  Collection<Region> getRegionsBySmsc(String smsc) {
    Collection<Region> result = new LinkedList<Region>();
    for(Region r : regions.values()) {
      if(r.getSmsc().equals(smsc)) {
        result.add(r);
      }
    }
    return result;
  }

}
