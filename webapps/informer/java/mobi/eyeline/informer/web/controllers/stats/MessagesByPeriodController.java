package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.DeliveryStatVisitor;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Locale;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByPeriodController extends DeliveryStatController implements DeliveryStatVisitor {


  private Delivery delivery = null;


  public MessagesByPeriodController() {
    super(new MessagesByPeriodTotals());
  }

  public Integer getDeliveryId() {
    return delivery != null ? delivery.getId() : null;
  }

  public String getDeliveryName() {
    Delivery d = getDelivery();
    return d == null ? null : d.getName();
  }



  public Delivery getDelivery() {
    String s = getRequestParameter("delivery");
    if (s != null) {
      try {
        int deliveryId = Integer.parseInt(s);
        boolean firstTime= (delivery == null || delivery.getId() != deliveryId);
        delivery = getConfig().getDelivery(getUser().getLogin(), getUser().getPassword(), deliveryId);
        getFilter().setFromDate(delivery.getStartDate());
        getFilter().setTaskId(deliveryId);
        if(firstTime) {
          reset();
          getFilter().setFromDate(delivery.getStartDate());
          getFilter().setTaskId(deliveryId);
          start();
        }
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return delivery;
  }


  public void clearFilter() {
    super.clearFilter();
    getFilter().setTaskId(getDeliveryId());
  }


  @Override
  public void loadRecords(final Configuration config, final Locale locale) throws AdminException {


    DeliveryStatFilter filterCopy = new DeliveryStatFilter(getFilter());
    if (delivery != null && filterCopy.getFromDate() == null) {
      filterCopy.setFromDate(delivery.getStartDate());
    }
    config.statistics(filterCopy, this);

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


}

