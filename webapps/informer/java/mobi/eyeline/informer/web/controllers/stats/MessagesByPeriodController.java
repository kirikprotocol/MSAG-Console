package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.web.config.Configuration;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Locale;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByPeriodController extends DeliveryStatController implements DeliveryStatVisitor {

  private DeliveryStatFilter filter;
  private Delivery delivery= null;


 


  public MessagesByPeriodController() {
    super();
    filter = new DeliveryStatFilter();
  }

  public Integer getDeliveryId() {
    Delivery d = getDelivery();
    return delivery!=null  ? delivery.getId() : null;
  }

  public String getDeliveryName() {
    Delivery d = getDelivery();
    return d==null ? null : d.getName();
  }

  public Delivery getDelivery() {
    String s = getRequestParameter("delivery");
    if(s!=null) {
      try {
        int deliveryId = Integer.parseInt(s);
        if(delivery!=null && delivery.getId()!=deliveryId) {
          reset();
        }
        delivery = getConfig().getDelivery(getUser().getLogin(),getUser().getPassword(),deliveryId);
        filter.setTaskId(deliveryId);
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return delivery;
  }




  public void clearFilter() {
    reset();
    clearRecords();
    filter.setUser(null);
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskId(getDeliveryId());
    detalised = false;
  }


  public DeliveryStatFilter getFilter() {
    return filter;
  }

  public void setFilter(DeliveryStatFilter filter) {
    this.filter = filter;
  }


  @Override
  public void loadRecords(final Configuration config, final Locale locale) throws AdminException {


      DeliveryStatFilter filterCopy = new DeliveryStatFilter(filter);
      if(delivery!=null && filterCopy.getFromDate()==null) {
        // todo remove comment
        // filterCopy.setFromDate(delivery.getStartDate());
      }
      config.statistics(filterCopy,this);

  }


  public boolean visit(DeliveryStatRecord rec, int total, int current) {

    setCurrentAndTotal(current,total);
    AggregatedRecord newRecord = new MessagesByPeriodRecord(rec,getAggregation(),true);
    AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
    if(oldRecord==null) {
      putRecord(newRecord);
    }
    else {
      oldRecord.add(newRecord);
    }
    return !isCancelled();
  }







}

