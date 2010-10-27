package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.DeliveryStatVisitor;

import java.util.Locale;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class DeliveryCommonStatController extends DeliveryStatController implements DeliveryStatVisitor {

  private DeliveryStatFilter filter;


  public DeliveryCommonStatController() {
    super();
    filter = new DeliveryStatFilter();
  }


  public void clearFilter() {
    filter.setUser(null);
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskId(null);
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


