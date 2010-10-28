package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;

import java.util.Locale;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class DeliveryCommonStatController extends DeliveryStatController implements DeliveryStatVisitor {

  private DeliveryStatFilter filter;
  private Delivery delivery= null;


  public DeliveryCommonStatController() {
    super();
    filter = new DeliveryStatFilter();
  }

  public Integer getDeliveryId() {
    String s = getRequestParameter("delivery");
    if(s!=null) {
      try {
        int deliveryId = Integer.parseInt(s);
        delivery = getConfig().getDelivery(getUser().getLogin(),getUser().getPassword(),deliveryId);
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return delivery!=null  ? delivery.getId() : null;

  }

  public void setDeliveryId(Integer id) {
     if(id!=null) {
      try {
        delivery = getConfig().getDelivery(getUser().getLogin(),getUser().getPassword(),id);
      }
      catch (AdminException e) {
        addError(e);
      }
    }
  }


  public void clearFilter() {
    filter.setUser(null);
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskId(getDeliveryId());
  }


  public DeliveryStatFilter getFilter() {
    return filter;
  }

  public void setFilter(DeliveryStatFilter filter) {
    this.filter = filter;
  }


  @Override
  public void loadRecords(final Locale locale) throws AdminException {
     getConfig().getDeliveryStatProvider().accept(filter,this);
  }


  public boolean visit(DeliveryStatRecord rec, int total, int current) {
    
//    Object l =new Object();
//    synchronized (l){
//      try {
//        l.wait(1000);
//      }
//      catch (InterruptedException e) {
//        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
//      }
//    }


    setCurrentAndTotal(current,total);
    AggregatedStatRecord newRecord = new AggregatedCommonStatRecord(rec,getAggregation(),true);
    AggregatedStatRecord oldRecord = getRecord(newRecord.getStartCalendar().getTime());
    if(oldRecord==null) {
      putRecord(newRecord);
    }
    else {
      oldRecord.add(newRecord);
    }
    return true;
  }




}


