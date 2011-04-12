package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.LongOperationController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByDeliveriesController extends LongOperationController {
  private User currentUser;
  private boolean initError = false;
  private MessagesByDeliveriesTotals totals;
  private List<MessagesByDeliveriesRecord> records;
  private DeliveryStatFilter filter;
  boolean fullMode = false;
  private boolean hideDeleted;

  private String nameFilter;

  private String regionId;

  public MessagesByDeliveriesController() {
    super();
    totals = new MessagesByDeliveriesTotals();
    records = Collections.synchronizedList(new ArrayList<MessagesByDeliveriesRecord>());
    filter = new DeliveryStatFilter();
    initUser();
    filter.setFromDate(getLastWeekStart().getTime());
    start();
  }

  protected static Calendar getLastWeekStart() {
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR_OF_DAY,0);
    c.set(Calendar.MINUTE,0);
    c.set(Calendar.SECOND,0);
    c.set(Calendar.MILLISECOND,0);
    c.add(Calendar.DATE,-7);
    return c;
  }

  private void initUser() {
    currentUser = getConfig().getUser(getUserName());
    if (currentUser == null) initError = true;
    else if (!currentUser.hasRole(User.INFORMER_ADMIN_ROLE)) {
      filter.setUser(currentUser.getLogin());
    } else {
      filter.setUser(null);
    }
  }

  public DeliveryStatFilter getFilter() {
    return filter;
  }

  public void clearFilter() {
    reset();
    totals.reset();
    records.clear();
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
    nameFilter = null;
    regionId = null;
    hideDeleted = false;
    fullMode = false;
  }

  public String getRegionId() {
    return regionId;
  }

  public void setRegionId(String regionId) {
    this.regionId = regionId;
  }

  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter;
  }

  public List<SelectItem> getRegions() {
    List<Region> rs = getConfig().getRegions();
    Collections.sort(rs, new Comparator<Object>() {
      @Override
      public int compare(Object o1, Object o2) {
        return ((Region)o1).getName().compareTo(((Region)o2).getName());
      }
    });
    List<SelectItem> sis = new ArrayList<SelectItem>(rs.size());
    sis.add(new SelectItem("0", ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale()).getString("region.default")));
    for(Region r : rs) {
      sis.add(new SelectItem(Integer.toString(r.getRegionId()), r.getName()));
    }
    return sis;
  }

  public User getCurrentUser() {
    return currentUser;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public boolean isHideDeleted() {
    return hideDeleted;
  }

  public void setHideDeleted(boolean hideDeleted) {
    this.hideDeleted = hideDeleted;
  }

  public boolean isAdmin() {
    return currentUser.hasRole(User.INFORMER_ADMIN_ROLE);
  }


  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(""));
    for (User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }


  @Override
  public void execute(final Configuration config, final Locale locale) throws InterruptedException, AdminException {
    totals.reset();
    records.clear();

    DeliveryStatFilter deliveryFilter = new DeliveryStatFilter();
    deliveryFilter.setFromDate(filter.getFromDate());
    deliveryFilter.setTillDate(filter.getTillDate());
    if (filter.getUser() != null) {
      deliveryFilter.setUser(filter.getUser());
    }

    final Map<Key, MessagesByDeliveriesRecord> recsMap = new HashMap<Key, MessagesByDeliveriesRecord>();

    config.statistics(deliveryFilter, new DeliveryStatVisitor() {

      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        setCurrentAndTotal(current, total);

        Key key = new Key(rec.getTaskId(), rec.getRegionId());
        MessagesByDeliveriesRecord r = recsMap.get(key);
        if (r == null) {
          User user = config.getUser(rec.getUser());

          Delivery delivery = null;
          if (user != null) {
            try {
              delivery = config.getDelivery(user.getLogin(), user.getPassword(), rec.getTaskId());
            } catch (AdminException e) {
              logger.error(e,e);
            }
          }

          r = new MessagesByDeliveriesRecord(rec.getUser(), rec.getTaskId(), locale);
          r.setDelivery(delivery);
          r.setUser(user);
          recsMap.put(key, r);
        }
        if(rec.getRegionId() != null) {
          r.setRegionId(rec.getRegionId());
          if(rec.getRegionId() != 0) {
          Region region = config.getRegion(rec.getRegionId());
          if(region != null) {
            r.setRegion(region.getName());
          }
          }else {
            r.setRegion(ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("region.default"));
          }
        }
        r.incNewMessages(rec.getNewmessages());
        r.incProcMessages(rec.getProcessing());
        r.incDeliveredMessages(rec.getDelivered());
        r.incDeliveredSms(rec.getDeliveredSMS());
        r.incFailedMessages(rec.getFailed());
        r.incFailedSms(rec.getFailedSMS());
        r.incExpiredMessages(rec.getExpired());
        r.incExpiredSms(rec.getExpiredSMS());
        r.updateTime(rec.getDate());

        return !isCancelled();
      }
    });

    for (MessagesByDeliveriesRecord r : recsMap.values()) {
      if (hideDeleted && r.getDelivery() == null)
        continue;
      if (nameFilter != null && nameFilter.trim().length() > 0 && (r.getDelivery() == null || !r.getDelivery().getName().contains(nameFilter)))
        continue;
      if(regionId != null && regionId.length()>0 && (r.getRegionId() == null || Integer.parseInt(regionId) != r.getRegionId())) {
        continue;
      }
      records.add(r);
      getTotals().add(r);
    }
  }

  public DataTableModel getRecords() {

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        if (sortOrder != null && !records.isEmpty()) {
          Collections.sort(records, records.get(0).getRecordsComparator(sortOrder));
        }

        List<MessagesByDeliveriesRecord> result = new LinkedList<MessagesByDeliveriesRecord>();
        for (Iterator<MessagesByDeliveriesRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          MessagesByDeliveriesRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return records.size();
      }
    };
  }

  public boolean isFullMode() {
    return fullMode;
  }

  public void setFullMode(boolean fullMode) {
    this.fullMode = fullMode;
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {

    for (int i = 0, recordsSize = records.size(); i < recordsSize; i++) {
      MessagesByDeliveriesRecord r = records.get(i);
      if (i == 0) r.printCSVheader(writer, fullMode);
      r.printWithChildrenToCSV(writer, fullMode);
    }
  }

  public MessagesByDeliveriesTotals getTotals() {
    return totals;
  }

  private static class Key {
    private int deliveryId;
    private Integer regionId;
    private Key(int deliveryId, Integer regionId) {
      this.deliveryId = deliveryId;
      this.regionId = regionId;
    }
    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (!(o instanceof Key)) return false;
      Key key = (Key) o;
      if (deliveryId != key.deliveryId) return false;
      if (regionId != null ? !regionId.equals(key.regionId) : key.regionId != null) return false;
      return true;
    }
    @Override
    public int hashCode() {
      int result = deliveryId;
      result = 31 * result + (regionId != null ? regionId.hashCode() : 0);
      return result;
    }
  }
}
