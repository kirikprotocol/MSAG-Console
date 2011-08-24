package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.*;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

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
public class MessagesByDeliveriesController extends InformerController{
  private User currentUser;
  private boolean initError = false;
  private MessagesByDeliveriesTotals totals;
  private List<MessagesByDeliveriesRecord> records;
  private DeliveryStatFilter filter;
  boolean fullMode = false;
  private boolean hideDeleted;

  private String nameFilter;

  private boolean init = true;

  private LoadListener loadListener;

  public MessagesByDeliveriesController() {
    super();
    totals = new MessagesByDeliveriesTotals();
    records = Collections.synchronizedList(new ArrayList<MessagesByDeliveriesRecord>());
    filter = new DeliveryStatFilter();
    initUser();
    filter.setFromDate(getLastWeekStart().getTime());
  }

  private boolean loaded;


  public boolean isLoaded() {
    return loaded;
  }

  public String start() {
    init = true;
    loaded = false;
    loadListener = null;
    return null;
  }

  public boolean isInit() {
    return init;
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
    loaded = false;
    loadListener = null;
    totals.reset();
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setRegionId(null);
    nameFilter = null;
    hideDeleted = false;
    fullMode = false;
  }

  public String getRegionId() {
    return filter.getRegionId() == null ? null : filter.getRegionId().toString();
  }

  public void setRegionId(String regionId) {
    filter.setRegionId(regionId == null || regionId.length() == 0 ? null : Integer.parseInt(regionId));
  }

  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter;
  }

  public List<SelectItem> getRegions() {
    List<Region> rs = getConfig().getRegions();
    User u = getConfig().getUser(getUserName());
    if(u == null || (!u.isAllRegionsAllowed() && u.getRegions() == null)) {
      return Collections.emptyList();
    }
    if(!u.isAllRegionsAllowed()) {
      Set<Integer> available = new HashSet<Integer>(u.getRegions());
      Iterator<Region> i = rs.iterator();
      while(i.hasNext()) {
        Region r = i.next();
        if(!available.contains(r.getRegionId())) {
          i.remove();
        }
      }
    }
    Collections.sort(rs, new Comparator<Object>() {
      @Override
      public int compare(Object o1, Object o2) {
        return ((Region)o1).getName().compareTo(((Region)o2).getName());
      }
    });
    List<SelectItem> sis = new ArrayList<SelectItem>(rs.size());
    if(u.isAllRegionsAllowed()) {
      sis.add(new SelectItem("0", ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale()).getString("region.default")));
    }
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
    Collections.sort(ret, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ret;
  }

  private boolean getRegion(Configuration config, Locale locale, Integer regId, String[] result) {
    if(regId != null) {
      if(regId != 0) {
        Region r = config.getRegion(regId);
        if(r != null) {
          result[0] = r.getName();
          return true;
        }
      }else {
        result[0] = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("region.default");
        return false;
      }
    }
    result[0] = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("stat.page.deletedRegion") +" (id="+regId+')';
    return false;
  }


  public void execute(final Configuration config, final Locale locale, final LoadListener listener) throws AdminException {
    totals.reset();
    records.clear();

    DeliveryStatFilter deliveryFilter = new DeliveryStatFilter();
    deliveryFilter.setFromDate(filter.getFromDate());
    deliveryFilter.setTillDate(filter.getTillDate());
    deliveryFilter.setRegionId(filter.getRegionId());
    if (filter.getUser() != null) {
      deliveryFilter.setUser(filter.getUser());
    }

    final Map<Integer, MessagesByDeliveriesRecord> recsMap = new HashMap<Integer, MessagesByDeliveriesRecord>();

    config.statistics(deliveryFilter, new DeliveryStatVisitorImpl(listener, recsMap, config, locale));

    for (MessagesByDeliveriesRecord r : recsMap.values()) {
      if (hideDeleted && r.isDeletedDelviery())
        continue;
      if (nameFilter != null && nameFilter.trim().length() > 0 && !(r.getDeliveryName().startsWith(nameFilter)))
        continue;
      records.add(r);
      getTotals().add(r);
    }
  }

  public DataTableModel getRecords() {

    final Locale locale = getLocale();

    final Configuration config = getConfig();

    return new PreloadableModel() {

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

      @Override
      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder sortOrder) {
        LoadListener listener = null;
        if(!loaded) {
          if(loadListener == null) {
            loadListener = new LoadListener();
            new Thread() {
              public void run() {
                try{
                  MessagesByDeliveriesController.this.execute(config, locale, loadListener);
                  loaded = true;
                }catch (AdminException e){
                  logger.error(e,e);
                  loadListener.setLoadError(new ModelException(e.getMessage(locale)));
                }catch (Exception e){
                  logger.error(e,e);
                }
              }
            }.start();
          }
          listener = loadListener;
        }
        return listener;
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
      if (i == 0) r.printCSVheader(writer);
      r.printWithChildrenToCSV(writer);
    }
  }

  public MessagesByDeliveriesTotals getTotals() {
    return totals;
  }

  private class DeliveryStatVisitorImpl implements DeliveryStatVisitor {

    private final LoadListener listener;
    private final Map<Integer, MessagesByDeliveriesRecord> recsMap;
    private final Configuration config;
    private final Locale locale;

    private Map<Integer, Delivery> deliveriesCache = new HashMap<Integer, Delivery>();

    public DeliveryStatVisitorImpl(LoadListener listener, Map<Integer, MessagesByDeliveriesRecord> recsMap, Configuration config, Locale locale) {
      this.listener = listener;
      this.recsMap = recsMap;
      this.config = config;
      this.locale = locale;
    }

    public boolean visit(DeliveryStatRecord rec, int total, int current) {
      listener.setCurrent(current);
      listener.setTotal(total);

      Integer key = rec.getTaskId();
      MessagesByDeliveriesRecord oldRecord = recsMap.get(key);
      String[] region = new String[1];
      String[] delivery = new String[1];
      int state = getDeliveryName(rec.getUser(), rec.getTaskId(), delivery);
      boolean archivated = state == 0;
      boolean deleted = state == -1;
      boolean deletedRegion = !getRegion(config, locale, rec.getRegionId(), region);
      if (oldRecord == null) {
        oldRecord = new MessagesByDeliveriesRecord(rec, config.getUser(rec.getUser()), delivery[0], deleted, archivated, region[0], deletedRegion, true);
        recsMap.put(key, oldRecord);
      }else {
        MessagesByDeliveriesRecord c = new MessagesByDeliveriesRecord(rec, config.getUser(rec.getUser()), delivery[0], deleted, archivated, region[0], deletedRegion, false);
        oldRecord.add(c);
      }
      oldRecord.updateTime(rec.getDate());
      oldRecord.setDeliveryStatus(getDeliveryStatus(rec.getUser(), rec.getTaskId()));

      return true;
    }

    private Delivery getDelivery(String login, int deliveryId) throws AdminException{
      Delivery d = deliveriesCache.get(deliveryId);
      if(d == null) {
        try {
          d = config.getDelivery(login, deliveryId);
        } catch (AdminException e) {
          logger.error(e,e);
        }
        if(d != null) {
          deliveriesCache.put(deliveryId, d);
        }
      }
      return d;
    }

    private int getDeliveryName(String userId, int deliveryId, String[] result) {
      User user = config.getUser(userId);
      Delivery delivery = null;
      if (user != null) {
        try {
          delivery = getDelivery(user.getLogin(), deliveryId);
        } catch (AdminException e) {
          logger.error(e,e);
        }
        if(delivery != null) {
          result[0] = delivery.getName();
          return 1;
        }else {
          if(config.isArchiveDaemonDeployed()) {
            try {
              delivery = config.getArchiveDelivery(user.getLogin(), deliveryId);
            } catch (AdminException e) {
              logger.error(e,e);
            }
          }
          if(delivery != null) {
            result[0] = delivery.getName()+" (" +
                ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("stat.page.archivedDelivery") +", id="+deliveryId+')';
            return 0;
          }
        }
      }
      result[0] = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("stat.page.deletedDelivery") +" (id="+deliveryId+')';
      return -1;
    }

    private String getDeliveryStatus(String userId, int deliveryId) {
      User user = config.getUser(userId);
      Delivery delivery = null;
      if (user != null) {
        try {
          delivery = getDelivery(user.getLogin(), deliveryId);
        } catch (AdminException e) {
          logger.error(e,e);
        }
        if(delivery != null) {
          return  ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("delivery.status."+delivery.getStatus());
        }
      }
      return "";
    }
  }
}
