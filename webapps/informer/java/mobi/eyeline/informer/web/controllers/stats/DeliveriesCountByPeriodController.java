package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryFields;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryInfo;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Calendar;
import java.util.Locale;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.10.2010
 * Time: 15:20:49
 */
public class DeliveriesCountByPeriodController extends DeliveryStatController implements Visitor<DeliveryInfo> {


  public DeliveriesCountByPeriodController() {
    super(new DeliveriesCountByPeriodTotals());
    Calendar c = getLastWeekStart();
    getFilter().setFromDate(c.getTime());
    start();
  }


  @Override
  public void loadRecords(Configuration config, final Locale locale) throws AdminException {
    DeliveryFilter f = new DeliveryFilter();
    String filterUser = getFilter().getUser();
    if (filterUser != null) {
      f.setUserIdFilter(filterUser);
    }
    f.setStartDateFrom(getFilter().getFromDate());
    f.setStartDateTo(getFilter().getTillDate());
    f.setResultFields(DeliveryFields.StartDate);

    config.getDeliveries(getUser().getLogin(), getUser().getPassword(), f, 1000, this);
  }

  public boolean visit(DeliveryInfo di) throws AdminException {
    Calendar c = Calendar.getInstance();
    c.setTime(di.getStartDate());
    AggregatedRecord newRecord = new DeliveriesCountByPeriodRecord(c, getAggregation(), 1, true);
    AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
    if (oldRecord == null) {
      putRecord(newRecord);
    } else {
      oldRecord.add(newRecord);
    }
    getTotals().add(newRecord);
    setCurrent(getCurrent() + 1);
    return !isCancelled();
  }

}