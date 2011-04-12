package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByPeriodController extends DeliveryStatController implements DeliveryStatVisitor {


  private List<Integer> deliveriesIds = null;

  private StorageStrategy strategy = new CommonStorageStrategy();


  public MessagesByPeriodController() {
    super(new MessagesByPeriodTotals());
    Calendar c = getLastWeekStart();
    getFilter().setFromDate(c.getTime());
    setDeliveryParam();
//    start();
  }

  public String getDeliveryName() {
    if(deliveriesIds==null) return null;
    if(deliveriesIds.size()!=1) return ""; //empty or multiple;
    try {
      return strategy.getDelivery(getUser().getLogin(), getUser().getPassword(), deliveriesIds.get(0)).getName();
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
  public String start() {
    clearRecords();
    return super.start();
  }

  @Override
  public void loadRecords(final Configuration config, final Locale locale) throws AdminException {
    this.locale = locale;
    config.statistics(getFilter(), this);
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

  public List<SelectItem> getRegions() {
    List<Region> rs = getConfig().getRegions();
    Collections.sort(rs, new Comparator<Object>() {
      @Override
      public int compare(Object o1, Object o2) {
        return ((Region)o1).getName().compareTo(((Region)o2).getName());
      }
    });
    List<SelectItem> sis = new ArrayList<SelectItem>(rs.size()+1);
    sis.add(new SelectItem("0", ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale()).getString("region.default")));
    for(Region r : rs) {
      sis.add(new SelectItem(Integer.toString(r.getRegionId()), r.getName()));
    }
    return sis;
  }
  public String getRegionId() {
    return filter.getRegionId() == null ? null : filter.getRegionId().toString();
  }

  public void setRegionId(String regionId) {
    filter.setRegionId(regionId == null || regionId.length() == 0 ? null : Integer.parseInt(regionId));
  }

  public boolean visit(DeliveryStatRecord rec, int total, int current) {
    AggregationType type = getAggregation();
    setCurrentAndTotal(current, total);
    AggregatedRecord newRecord = type != AggregationType.REGION ? createWithTimeAggregation(rec) : createWithRegionAggregation(rec);
    AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
    if (oldRecord == null) {
      putRecord(newRecord);
    } else {
      oldRecord.add(newRecord);
    }
    getTotals().add(newRecord);
    return !isCancelled();
  }


  public String setDeliveryParam() {
    if(Boolean.valueOf(getRequestParameter("archive")) || Boolean.valueOf((String)getRequest().get("archive"))) {
      strategy = new ArchiveStorageStrategy();
    }else {
      strategy = new CommonStorageStrategy();
    }
    String s = getRequestParameter("delivery");
    if (s == null)
      s = (String)getRequest().get("delivery");
    if (s != null) {
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
          for(int id : newIds) {
            if(!deliveriesIds.contains(id)) {
              firstTime = true;
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
            Delivery d = strategy.getDelivery(getUser().getLogin(), getUser().getPassword(), id);
            Date startDate = d.getStartDate();
            if(from.after(startDate)) {
              from = startDate;
            }
          }
          getFilter().setFromDate(from);

          reset();
          start();
        }
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return "STATS_DELIVERY_MESSAGES_BY_PERIOD";
  }


  private interface StorageStrategy {
    Delivery getDelivery(String login, String password, int id) throws AdminException;
  }

  private class CommonStorageStrategy implements StorageStrategy {
    @Override
    public Delivery getDelivery(String login, String password, int id) throws AdminException {
      return getConfig().getDelivery(login, password, id);
    }
  }

  private class ArchiveStorageStrategy implements StorageStrategy {
    @Override
    public Delivery getDelivery(String login, String password, int id) throws AdminException {
      return getConfig().getArchiveDelivery(login, id);
    }
  }
}

