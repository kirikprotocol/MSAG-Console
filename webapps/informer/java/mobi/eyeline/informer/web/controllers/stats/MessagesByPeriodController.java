package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.web.config.Configuration;

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
  public void loadRecords(final Configuration config, final Locale locale) throws AdminException {
    config.statistics(getFilter(), this);
  }


  public boolean visit(DeliveryStatRecord rec, int total, int current) {
    setCurrentAndTotal(current, total);
    AggregatedRecord newRecord = new MessagesByPeriodRecord(rec, getAggregation(), true);
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

