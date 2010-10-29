package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Locale;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByUserStatsController extends DeliveryStatController {


  private DeliveryStatFilter filter;
  private Delivery delivery= null;

  public boolean isDetaliseBySMS() {
    return detaliseBySMS;
  }

  public void setDetaliseBySMS(boolean detaliseBySMS) {
    this.detaliseBySMS = detaliseBySMS;
  }

  boolean detaliseBySMS=false;


  public MessagesByUserStatsController() {
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
    detaliseBySMS = false;
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
      filterCopy.setFromDate(delivery.getStartDate());
    }

      //todo bySMS detailed stat


    DeliveryFilter deliveryFilter = new DeliveryFilter();
    deliveryFilter.setStartDateFrom(filter.getFromDate());
    deliveryFilter.setEndDateTo(filter.getTillDate());
    if(filter.getUser()!=null) {
      deliveryFilter.setUserIdFilter(new String[]{filter.getUser()});
    }

    setCurrentAndTotal(0,config.countDeliveries(getUser().getLogin(),getUser().getPassword(),deliveryFilter));

    deliveryFilter.setResultFields(new DeliveryFields[]{DeliveryFields.UserId,DeliveryFields.StartDate,DeliveryFields.EndDate});

    config.getDeliveries(getUser().getLogin(),getUser().getPassword(),deliveryFilter,1000,
        new Visitor<DeliveryInfo>() {
          public boolean visit(DeliveryInfo deliveryInfo ) throws AdminException {
            final int deliveryId = deliveryInfo.getDeliveryId();


            MessageFilter messageFilter = new MessageFilter(
                deliveryId,
                filter.getFromDate()==null ? deliveryInfo.getStartDate() : filter.getFromDate(),
                filter.getTillDate()==null ? deliveryInfo.getEndDate() : filter.getTillDate()
            );

            messageFilter.setFields(new MessageFields[]{MessageFields.Text});

            DeliveryStatistics stat = config.getDeliveryStats(getUser().getLogin(),getUser().getPassword(),deliveryId);

            MessagesByUserStatRecord rec = new MessagesByUserStatRecord(deliveryInfo.getUserId(),stat);
            putRecord(rec);

            setCurrent(getCurrent()+1);
            return !isCancelled();
          }
        }
    );


  }







}