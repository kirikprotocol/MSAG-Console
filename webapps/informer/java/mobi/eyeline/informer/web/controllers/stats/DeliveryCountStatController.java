package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Calendar;
import java.util.Locale;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.10.2010
 * Time: 15:20:49
 */
public class DeliveryCountStatController  extends DeliveryStatController implements Visitor<DeliveryInfo> {

  private DeliveryStatFilter filter;


  public DeliveryCountStatController() {
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
  public void loadRecords(Configuration config, final Locale locale) throws AdminException{
      DeliveryFilter f = new DeliveryFilter();
      String filterUser = filter.getUser();
      if(filterUser!=null) {
        f.setUserIdFilter(new String[]{filterUser});
      }
      f.setStartDateFrom(filter.getFromDate());
      f.setStartDateTo(filter.getTillDate());
      f.setResultFields(new DeliveryFields[]{DeliveryFields.StartDate});

      config.getDeliveries(getUser().getLogin(), getUser().getPassword(), f, 1000, this);
  }

  public boolean visit(DeliveryInfo di) throws AdminException {
    Calendar c = Calendar.getInstance();
    c.setTime(di.getStartDate());
    AggregatedStatRecord newRecord = new AggregatedCountStatRecord(c,getAggregation(),1,true);
    AggregatedStatRecord oldRecord = getRecord(newRecord.getStartCalendar().getTime());
    if(oldRecord==null) {
      putRecord(newRecord);
    }
    else {
      oldRecord.add(newRecord);
    }
    setCurrent(getCurrent()+1);
    return !isCancelled();
  }
  
}