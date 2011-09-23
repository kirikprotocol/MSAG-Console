package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Route implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Route.class);

  private String name = null;
  private String notes = "";

  private int priority = 0;

  private String srcSmeId;
  private List<Source> sources = new ArrayList<Source>();
  private List<Destination> destinations = new ArrayList<Destination>();

  private BillingMode billing = null;

  private int serviceId = 0;
  private Integer aclId;
  private Long providerId;
  private Long categoryId;
  private String backupSmeId;

//  private boolean enabling = true;
  private TrafficMode trafficMode = TrafficMode.ALL;
  private boolean active;
  private boolean archiving = true;
  private boolean transit = false;
  private boolean suppressDeliveryReports;
  private DeliveryMode deliveryMode;
  private String forwardTo ;
  private boolean hide;
  private ReplayPath replayPath = ReplayPath.PASS;
  private boolean forceDelivery;
  private boolean allowBlocked;

  public Route(String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public Route() {
    
  }

  public String getName() {
    return name;
  }

  public void setName(String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public String getNotes() {
    return notes;
  }

  public void setNotes(String notes) {
    this.notes = notes;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) throws AdminException {
    vh.checkGreaterOrEqualsTo("priority", priority, 0);
    this.priority = priority;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public void setSrcSmeId(String srcSmeId) {
    this.srcSmeId = srcSmeId;
  }

  public void addSource(Source source) throws AdminException {
    vh.checkNotNull("sources", source);
    sources.add(new Source(source));
  }

  public boolean removeSource(String subject) {
    for (Iterator<Source> iter  = sources.iterator(); iter.hasNext();) {
      Source s = iter.next();
      if (s.getSubject() != null && s.getSubject().equals(subject)) {
        iter.remove();
        return true;
      }
    }
    return false;
  }

  public boolean removeSource(Address mask) {
    for (Iterator<Source> iter  = sources.iterator(); iter.hasNext();) {
      Source s = iter.next();
      if (s.getMask() != null && s.getMask().equals(mask)) {
        iter.remove();
        return true;
      }
    }
    return false;
  }

  public boolean containsSource(String subject) {
    for (Source s : sources) {
      if (s.getSubject() != null && s.getSubject().equals(subject))
        return true;
    }
    return false;
  }

  public boolean containsSource(Address mask) {
    for (Source s : sources) {
      if (s.getMask() != null && s.getMask().equals(mask))
        return true;
    }
    return false;
  }

  public List<Source> getSources() {
    return new ArrayList<Source>(sources);
  }

  public void setSources(List<Source> sources) throws AdminException {
    vh.checkNotNull("sources", sources);
    vh.checkNoNulls("sources", sources);
    this.sources = sources;
  }

  public boolean containsDestination(String subject) {
    for (Destination s : destinations) {
      if (s.getSubject() != null && s.getSubject().equals(subject))
        return true;
    }
    return false;
  }

  public void addDestination(Destination dst) throws AdminException {
    vh.checkNotNull("destinations", dst);
    destinations.add(dst);
  }

  public boolean removeDestination(String subject) {
    for (Iterator<Destination> iter  = destinations.iterator(); iter.hasNext();) {
      Destination s = iter.next();
      if (s.getSubject() != null && s.getSubject().equals(subject)) {
        iter.remove();
        return true;
      }
    }
    return false;
  }

  public boolean removeDestination(Address mask) {
    for (Iterator<Destination> iter  = destinations.iterator(); iter.hasNext();) {
      Destination s = iter.next();
      if (s.getMask() != null && s.getMask().equals(mask)) {
        iter.remove();
        return true;
      }
    }
    return false;
  }

  public boolean containsDestination(Address mask) {
    for (Destination s : destinations) {
      if (s.getMask() != null && s.getMask().equals(mask))
        return true;
    }
    return false;
  }

  public List<Destination> getDestinations() {
    return new ArrayList<Destination>(destinations);
  }

  public void setDestinations(List<Destination> destinations) throws AdminException {
    vh.checkNotNull("destinations", destinations);
    vh.checkNoNulls("destinations", destinations);
    this.destinations = destinations;
  }

  public BillingMode getBilling() {
    return billing;
  }

  public void setBilling(BillingMode billing) {
    this.billing = billing;
  }

  public int getServiceId() {
    return serviceId;
  }

  public void setServiceId(int serviceId) {
    this.serviceId = serviceId;
  }

  public Integer getAclId() {
    return aclId;
  }

  public void setAclId(Integer aclId) {
    this.aclId = aclId;
  }

  public Long getProviderId() {
    return providerId;
  }

  public void setProviderId(Long providerId) {
    this.providerId = providerId;
  }

  public Long getCategoryId() {
    return categoryId;
  }

  public void setCategoryId(Long categoryId) {
    this.categoryId = categoryId;
  }

  public String getBackupSmeId() {
    return backupSmeId;
  }

  public void setBackupSmeId(String backupSmeId) {
    this.backupSmeId = backupSmeId;
  }

  public TrafficMode getTrafficMode() {
    return trafficMode;
  }

  public void setTrafficMode(TrafficMode trafficMode) throws AdminException {
    vh.checkNotNull("trafficMode", trafficMode);
    this.trafficMode = trafficMode;
  }

  public boolean isActive() {
    return active;
  }

  public void setActive(boolean active) {
    this.active = active;
  }

  public boolean isArchiving() {
    return archiving;
  }

  public void setArchiving(boolean archiving) {
    this.archiving = archiving;
  }

  public boolean isTransit() {
    return transit;
  }

  public void setTransit(boolean transit) {
    this.transit = transit;
  }

  public boolean isSuppressDeliveryReports() {
    return suppressDeliveryReports;
  }

  public void setSuppressDeliveryReports(boolean suppressDeliveryReports) {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public DeliveryMode getDeliveryMode() {
    return deliveryMode;
  }

  public void setDeliveryMode(DeliveryMode deliveryMode) {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo() {
    return forwardTo;
  }

  public void setForwardTo(String forwardTo) {
    this.forwardTo = forwardTo;
  }

  public boolean isHide() {
    return hide;
  }

  public void setHide(boolean hide) {
    this.hide = hide;
  }

  public ReplayPath getReplayPath() {
    return replayPath;
  }

  public void setReplayPath(ReplayPath replayPath) {
    this.replayPath = replayPath;
  }

  public boolean isForceDelivery() {
    return forceDelivery;
  }

  public void setForceDelivery(boolean forceDelivery) {
    this.forceDelivery = forceDelivery;
  }

  public boolean isAllowBlocked() {
    return allowBlocked;
  }

  public void setAllowBlocked(boolean allowBlocked) {
    this.allowBlocked = allowBlocked;
  }
}
