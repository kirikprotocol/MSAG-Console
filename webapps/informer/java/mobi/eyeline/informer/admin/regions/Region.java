package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.io.Serializable;
import java.util.*;

/**
 * Регион
 *
 * @author Aleksandr Khalitov
 */
public class Region implements Serializable {

  private static transient final ValidationHelper vh = new ValidationHelper(Region.class);

  private Integer regionId;

  private String name;

  private String smsc;

  private TimeZone timeZone;

  private int maxSmsPerSecond;

  private final Collection<Address> masks = new LinkedList<Address>();

  void setRegionId(Integer regionId) {
    this.regionId = regionId;
  }

  void validate() throws AdminException{
    vh.checkNotEmpty("name", name);
    vh.checkNotEmpty("smsc", smsc);
    vh.checkNotNull("timeZone", timeZone);
    vh.checkGreaterOrEqualsTo("maxSmsPerSecond", maxSmsPerSecond, 0);

    for (Address ad : masks) {
      vh.checkNotNull("masks", ad);
      for(Address ad2 : masks) {
        vh.checkNotNull("masks", ad2);
        if (ad != ad2) {
          vh.checkNotEquals("masks", ad.getSimpleAddress(), ad2.getSimpleAddress());
        }
      }
    }
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Integer getRegionId() {
    return regionId;
  }

  public String getSmsc() {
    return smsc;
  }

  public void setSmsc(String smsc) {
    this.smsc = smsc;
  }

  public TimeZone getTimeZone() {
    return timeZone;
  }

  public void setTimeZone(TimeZone timeZone) {
    this.timeZone = timeZone;
  }

  public int getMaxSmsPerSecond() {
    return maxSmsPerSecond;
  }

  public void setMaxSmsPerSecond(int maxSmsPerSecond) {
    this.maxSmsPerSecond = maxSmsPerSecond;
  }

  public void addMask(Address a)  {
    if(a != null) {
      masks.add(a);
    }
  }

  public void addMasks(Collection<Address> masks) {
    this.masks.addAll(masks);
  }

  public void removeMask(Address r) {
    if (r != null)
      masks.remove(r);
  }

  public void clearMasks() {
    masks.clear();
  }

  public Collection<Address> getMasks() {
    return new ArrayList<Address>(masks);
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Region region = (Region) o;

    if (regionId != null ? !regionId.equals(region.regionId) : region.regionId != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    return regionId != null ? regionId.hashCode() : 0;
  }

  public Region cloneRegion() {
    Region r = new Region();
    r.regionId = regionId;
    r.smsc = smsc;
    r.maxSmsPerSecond = maxSmsPerSecond;
    r.name = name;
    r.timeZone = timeZone == null ? null : TimeZone.getTimeZone(timeZone.getID());
    for (Address a : masks) {
      r.masks.add(new Address(a));
    }
    return r;
  }

}
