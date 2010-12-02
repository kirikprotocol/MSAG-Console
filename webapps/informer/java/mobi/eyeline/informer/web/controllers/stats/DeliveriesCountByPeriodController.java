package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.delivery.stat.UserStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.UserStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.UserStatVisitor;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Calendar;
import java.util.Locale;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.10.2010
 * Time: 15:20:49
 */
public class DeliveriesCountByPeriodController extends DeliveryStatController implements UserStatVisitor {


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

    UserStatFilter filter = new UserStatFilter();
    filter.setUser(getFilter().getUser());
    filter.setFromDate(getFilter().getFromDate());
    filter.setTillDate(getFilter().getTillDate());

    config.statisticByUsers(filter, this);
  }

  public boolean visit(UserStatRecord rec, int total, int current) {
    Calendar c = Calendar.getInstance();
    c.setTime(rec.getDate());
    AggregatedRecord newRecord = new DeliveriesCountByPeriodRecord(c, getAggregation(), rec.getCreated(), true);
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