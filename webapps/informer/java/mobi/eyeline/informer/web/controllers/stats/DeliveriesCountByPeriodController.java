package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.stat.UserStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.UserStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.UserStatVisitor;
import mobi.eyeline.informer.web.components.data_table.model.LoadListener;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.10.2010
 * Time: 15:20:49
 */
public class DeliveriesCountByPeriodController extends DeliveryStatController{

  public DeliveriesCountByPeriodController() {
    super(new DeliveriesCountByPeriodTotals());
    Calendar c = getLastWeekStart();
    getFilter().setFromDate(c.getTime());

  }

  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for (AggregationType a : AggregationType.values()) {
      switch (a) {
        case SMSC: break;
        case REGION: break;
        default:
          ret.add(new SelectItem(a));
      }
    }
    return ret;
  }


  @Override
  public void loadRecords(Configuration config, final Locale locale, final LoadListener listener) throws AdminException {
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

    config.statisticByUsers(filter, new UserStatVisitorImpl(listener));
  }


  private class UserStatVisitorImpl implements UserStatVisitor {

    private final LoadListener listener;

    public UserStatVisitorImpl(LoadListener listener) {
      this.listener = listener;
    }

    public boolean visit(UserStatRecord rec, int total, int current) {
      listener.setCurrent(current);
      listener.setTotal(total);
      if (rec.getCreated() > 0) {
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
      }
      return true;
    }

  }
}