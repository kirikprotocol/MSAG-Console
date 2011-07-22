package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.LoadListener;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.text.MessageFormat;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByPeriodController extends DeliveryStatController{


  private List<Integer> deliveriesIds = null;

  private StorageStrategy strategy = new CommonStorageStrategy();

  private String smscFilter;

  public static final String COME_BACK_PARAMS = "stats_come_back_params";
  public static final String COME_BACK = "stats_come_back";

  public MessagesByPeriodController() {
    super(new MessagesByPeriodTotals());
    Calendar c = getLastWeekStart();
    getFilter().setFromDate(c.getTime());
    setDeliveryParam();
//    start();
  }

  @Override
  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for (AggregationType a : AggregationType.values()) {
      if(a != AggregationType.SMSC  || isUserInAdminRole()) {
        ret.add(new SelectItem(a));
      }
    }
    return ret;
  }

  public String getDeliveryName() {
    if(deliveriesIds==null) return null;
    if(deliveriesIds.size()!=1) return ""; //empty or multiple;
    try {
      return strategy.getDelivery(getUser().getLogin(), deliveriesIds.get(0)).getName();
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }


  public List<Integer> getDeliveriesIds() {
    return deliveriesIds;
  }

  public void clearFilter() {
    super.clearFilter();
    if(deliveriesIds!=null) {
      List<Integer> taskIds = new ArrayList<Integer>();
      taskIds.addAll(deliveriesIds);
      getFilter().setTaskIds(taskIds);
    }    
  }

  @Override
  public void loadRecords(final Configuration config, final Locale locale, final LoadListener l) throws AdminException {
    this.locale = locale;
    config.statistics(getFilter(), new DeliveryStatVisitorImpl(l));
  }

  private Locale locale;

  private AggregatedRecord createWithTimeAggregation(DeliveryStatRecord rec) {
    return new MessagesByPeriodRecord(rec, getAggregation(), true);
  }

  private AggregatedRecord createWithRegionAggregation(DeliveryStatRecord rec) {
    if(rec.getRegionId() != null) {
      if(rec.getRegionId() != 0) {
        Region r = getConfig().getRegion(rec.getRegionId());
        if(r != null) {
          return  new MessagesByRegionRecord(rec, r.getName(), false);
        }
      }else {
        return new MessagesByRegionRecord(rec, ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("region.default"), true);
      }
    }
    return new MessagesByRegionRecord(rec,
        ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("stat.page.deletedRegion") +" (id="+rec.getRegionId()+")",
        true);
  }

  private AggregatedRecord createWithSmscAggregation(DeliveryStatRecord rec) {
    String[] name = new String[]{null};
    boolean exist = getSmsc(rec, name);
    return new MessagesBySmscRecord(rec, name[0], !exist);
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
    List<SelectItem> sis = new ArrayList<SelectItem>(rs.size()+1);
    if(u.isAllRegionsAllowed()) {
      sis.add(new SelectItem("0", ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale()).getString("region.default")));
    }
    for(Region r : rs) {
      sis.add(new SelectItem(Integer.toString(r.getRegionId()), r.getName()));
    }
    return sis;
  }

  public List<SelectItem> getSmscs() {
    List<Smsc> rs = getConfig().getSmscs();
    Collections.sort(rs, new Comparator<Object>() {
      @Override
      public int compare(Object o1, Object o2) {
        return ((Smsc)o1).getName().compareTo(((Smsc)o2).getName());
      }
    });
    List<SelectItem> sis = new ArrayList<SelectItem>(rs.size());
    for(Smsc r : rs) {
      sis.add(new SelectItem(r.getName(), r.getName()));
    }
    return sis;
  }


  public String getRegionId() {
    return filter.getRegionId() == null ? null : filter.getRegionId().toString();
  }

  public void setRegionId(String regionId) {
    filter.setRegionId(regionId == null || regionId.length() == 0 ? null : Integer.parseInt(regionId));
  }

  public String getSmscFilter() {
    return smscFilter;
  }

  public void setSmscFilter(String smscFilter) {
    this.smscFilter = smscFilter;
  }

  private boolean getSmsc(DeliveryStatRecord record, String[] result) {
    if(record.getSmsc() != null) {
      boolean exist = getConfig().getSmsc(record.getSmsc()) != null;
      result[0] = exist ? record.getSmsc() :
          MessageFormat.format
              (
                  ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("stat.page.deletedSmsc"),
                  record.getSmsc()
              );
      return exist;
    }
    if(record.getRegionId() != null) {
      if(record.getRegionId() != 0) {
        Region r = getConfig().getRegion(record.getRegionId());
        if(r != null) {
          result[0] = r.getSmsc();
          return true;
        }
      }else {
        result[0] = getConfig().getDefaultSmsc();
        return true;
      }
    }
    result[0] = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("stat.page.unknownSmsc");
    return false;
  }


  private String backParams;
  private String backAction;

  public String getBackParams() {
    return backParams;
  }

  public String backAction() {
    return getBackAction();
  }

  public void setBackParams(String backParams) {
    this.backParams = backParams;
  }

  public String getBackAction() {
    return backAction == null  || backAction.length() == 0 ? null : backAction;
  }

  public void setBackAction(String backAction) {
    this.backAction = backAction;
  }

  public String setDeliveryParam() {

    backParams = getRequestParameter(COME_BACK_PARAMS);
    backAction = getRequestParameter(COME_BACK);

    if(Boolean.valueOf(getRequestParameter("archive")) || Boolean.valueOf((String)getRequest().get("archive"))) {
      strategy = new ArchiveStorageStrategy();
    }else {
      strategy = new CommonStorageStrategy();
    }
    String s = getRequestParameter("delivery");
    if (s == null)
      s = (String)getRequest().get("delivery");
    if (s != null && s.length() > 0) {
      try {

        StringTokenizer st = new StringTokenizer(s,",; ");
        List<Integer> newIds = new ArrayList<Integer>();
        while(st.hasMoreTokens()) {
          String token = st.nextToken().trim();
          if(token.length()>0) {
            newIds.add(Integer.parseInt(token));
          }
        }

        boolean firstTime=false;
        if(deliveriesIds==null) {
          firstTime = true;
        }
        else {
          if(deliveriesIds.size() != newIds.size()) {
            firstTime = true;
          }else {
            for(int id : newIds) {
              if(!deliveriesIds.contains(id)) {
                firstTime = true;
              }
            }
          }
        }

        if(firstTime) {
          deliveriesIds = newIds;

          List<Integer> taskIds = new ArrayList<Integer>();
          taskIds.addAll(newIds);
          getFilter().setTaskIds(taskIds);
          Date from = new Date();
          for(Integer id : deliveriesIds) {
            Delivery d = strategy.getDelivery(getUser().getLogin(), id);
            Date date = d.getCreateDate();
            if(from.after( date)) {
              from =  date;
            }
          }
          getFilter().setFromDate(from);

          clearRecords();
          start();
        }
      }
      catch (AdminException e) {
        addError(e);
      }
      return "STATS_DELIVERY_MESSAGES_BY_PERIOD";
    } else
      return null;

  }


  public boolean isFromDeliveriesList() {
    return deliveriesIds != null && (backAction == null || backAction.length() == 0);
  }


  private interface StorageStrategy {
    Delivery getDelivery(String login, int id) throws AdminException;
  }

  private class CommonStorageStrategy implements StorageStrategy {
    @Override
    public Delivery getDelivery(String login, int id) throws AdminException {
      return getConfig().getDelivery(login, id);
    }
  }

  private class ArchiveStorageStrategy implements StorageStrategy {
    @Override
    public Delivery getDelivery(String login, int id) throws AdminException {
      return getConfig().getArchiveDelivery(login, id);
    }
  }

  private class DeliveryStatVisitorImpl implements DeliveryStatVisitor {

    private final LoadListener l;

    public DeliveryStatVisitorImpl(LoadListener l) {
      this.l = l;
    }

    public boolean visit(DeliveryStatRecord rec, int total, int current) {
      AggregationType type = getAggregation();
      l.setCurrent(current);
      l.setTotal(total);
      AggregatedRecord newRecord;
      switch (type) {
        case REGION: newRecord = createWithRegionAggregation(rec); break;
        case SMSC:   newRecord = createWithSmscAggregation(rec); break;
        default:     newRecord = createWithTimeAggregation(rec); break;
      }

      if(smscFilter != null && smscFilter.length() > 0) {
        String[] smsc = new String[]{null};
        getSmsc(rec, smsc);
        if(!smsc[0].equals(smscFilter)) {
          return true;
        }
      }

      AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
      if (oldRecord == null) {
        putRecord(newRecord);
      } else {
        oldRecord.add(newRecord);
      }
      getTotals().add(newRecord);
      return true;
    }
  }
}

