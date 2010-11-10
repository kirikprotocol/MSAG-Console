package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.util.*;

/**
 * Регион
 * @author Aleksandr Khalitov
 */
public class Region {

  private Integer regionId;

  private String name;

  private String smsc;

  private TimeZone timeZone;

  private int maxSmsPerSecond;

  private final ValidationHelper vh = new ValidationHelper(Region.class);

  private Collection<Address> masks = new LinkedList<Address>();

  void setRegionId(Integer regionId) {
    this.regionId = regionId;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) throws AdminException{
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public Integer getRegionId() {
    return regionId;
  }

  public String getSmsc() {
    return smsc;
  }

  public void setSmsc(String smsc) throws AdminException {
    vh.checkNotEmpty("smsc", smsc);
    this.smsc = smsc;
  }

  public TimeZone getTimeZone() {
    return timeZone;
  }

  public void setTimeZone(TimeZone timeZone) throws AdminException {
    vh.checkNotNull("timeZone", timeZone);
    this.timeZone = timeZone;
  }

  public int getMaxSmsPerSecond() {
    return maxSmsPerSecond;
  }

  public void setMaxSmsPerSecond(int maxSmsPerSecond) throws AdminException{
    vh.checkGreaterThan("maxSmsPerSecond", maxSmsPerSecond, 0);
    this.maxSmsPerSecond = maxSmsPerSecond;
  }

  public void addMask(Address a) throws AdminException{
    vh.checkNotNull("masks", a);
    for(Address ad : masks) {
      if(ad.getSimpleAddress().equals(a.getSimpleAddress())) {
        return;
      }
    }
    masks.add(a);
  }

  public void removeMask(Address r) throws AdminException{
    Iterator<Address> i = masks.iterator();
    while(i.hasNext()) {
      Address a = i.next();
      if(a.getSimpleAddress().equals(r.getSimpleAddress())) {
        i.remove();
        break;
      }
    }
  }

  public void clearMasks() {
    masks.clear();
  }

  public Collection<Address> getMasks() {
    return new ArrayList<Address>(masks);
  }

  public Region cloneRegion() {
    Region r = new Region();
    r.regionId = regionId;
    r.smsc = smsc;
    r.maxSmsPerSecond = maxSmsPerSecond;
    r.name = name;
    r.timeZone = timeZone == null ? null : TimeZone.getTimeZone(timeZone.getID());
    for(Address a : masks) {
      r.masks.add(new Address(a.getTone(), a.getNpi(), a.getAddress()));
    }
    return r;
  }
  
}
