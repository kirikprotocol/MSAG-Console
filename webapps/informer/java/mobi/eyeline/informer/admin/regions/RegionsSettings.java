package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author Aleksandr Khalitov
 */
class RegionsSettings {

  private final Map<String, Region> regionsByMasks = new HashMap<String, Region>();
  private final Map<Integer, Region> regions = new LinkedHashMap<Integer, Region>(); // todo Почему здесь используется LinkedHashMap а не HashMap ?

  private int defaultMaxPerSecond;

  private final AtomicInteger lastId;

  RegionsSettings(Collection<Region> regions, int defaultMaxPerSecond, int lastId) {
    for (Region r : regions) {
      this.regions.put(r.getRegionId(), r);
      for (Address a : r.getMasks()) {
        regionsByMasks.put(a.getSimpleAddress(), r);
      }
    }
    this.defaultMaxPerSecond = defaultMaxPerSecond;
    this.lastId = new AtomicInteger(lastId);
  }

  RegionsSettings() {
    this.defaultMaxPerSecond = 0;
    this.lastId = new AtomicInteger(1);
  }

  private void checkMask(Region region) throws AdminException {
    for (Address a : region.getMasks()) {
      Region exist = regionsByMasks.get(a.getSimpleAddress());
      if (exist != null && !exist.getRegionId().equals(region.getRegionId())) {
        throw new RegionException("masks_intersection", a.getSimpleAddress());
      }
    }
  }

  private void checkName(Region region) throws AdminException {
    for (Region r : regions.values()) {
      if (!r.equals(region) && r.getName().equals(region.getName()))
        throw new RegionException("dublicate_name");
    }
  }

  void addRegion(Region region) throws AdminException {
    region.validate();
    checkMask(region);
    checkName(region);
    region.setRegionId(lastId.incrementAndGet());
    for (Address a : region.getMasks()) {
      regionsByMasks.put(a.getSimpleAddress(), region);
    }
    regions.put(region.getRegionId(), region);
  }

  void removeRegion(Integer regionId) {
    Region r = regions.remove(regionId);
    if (r != null) {
      for (Address a : r.getMasks()) {
        regionsByMasks.remove(a.getSimpleAddress());
      }
    }
  }

  void updateRegion(Region region) throws AdminException {
    region.validate();
    checkMask(region);
    checkName(region);

    Region old = regions.remove(region.getRegionId());
    if (old == null) {
      throw new RegionException("region_not_exist", region.getName());
    }
    regions.put(region.getRegionId(), region);
    for (Address a : old.getMasks()) {
      regionsByMasks.remove(a.getSimpleAddress());
    }
    for (Address a : region.getMasks()) {
      regionsByMasks.put(a.getSimpleAddress(), region);
    }
  }


  Region getRegion(Integer id) {
    return regions.get(id);
  }

  Region getRegionByAddress(Address address) {
    String mask = address.getSimpleAddress();
    Region r = regionsByMasks.get(mask);
    while (r == null && !mask.startsWith("?")) {
      int i = mask.indexOf("?");
      if (i > 0) {
        mask = mask.substring(0, i - 1) + '?' + mask.substring(i);
      } else if (i == -1) {
        mask = mask.substring(0, mask.length() - 1) + '?';
      }
      r = regionsByMasks.get(mask);
    }
    return r;
  }


  List<Region> getRegions() {
    return new ArrayList<Region>(regions.values());
  }

  Map<Integer, Region> getRegionsMap() {
    return new HashMap<Integer, Region>(regions);
  }


  Collection<Region> getRegionsBySmsc(String smsc) {
    Collection<Region> result = new LinkedList<Region>();
    for (Region r : regions.values()) {
      if (r.getSmsc().equals(smsc)) {
        result.add(r);
      }
    }
    return result;
  }


  int getDefaultMaxPerSecond() {
    return defaultMaxPerSecond;
  }

  void setDefaultMaxPerSecond(int defaultMaxPerSecond) throws AdminException {
    if (defaultMaxPerSecond <= 0) {
      throw new RegionException("def_region_incorrect");
    }
    this.defaultMaxPerSecond = defaultMaxPerSecond;
  }
}
