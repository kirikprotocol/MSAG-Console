package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.LoadListener;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Calendar;
import java.util.Locale;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByUserStatsController extends DeliveryStatController {


  public MessagesByUserStatsController() {
    super(new MessagesByUserStatTotals());
    Calendar c = getLastWeekStart();
    getFilter().setFromDate(c.getTime());
  }


  @Override
  public void loadRecords(final Configuration config, final Locale locale, final LoadListener listener) throws AdminException {
    DeliveryStatFilter filterCopy = new DeliveryStatFilter(getFilter());

    config.statistics(filterCopy, new DeliveryStatVisitorImpl(listener, config));
  }


  private class DeliveryStatVisitorImpl implements DeliveryStatVisitor {

    private final LoadListener listener;
    private final Configuration config;

    public DeliveryStatVisitorImpl(LoadListener listener, Configuration config) {
      this.listener = listener;
      this.config = config;
    }

    public boolean visit(DeliveryStatRecord rec, int total, int current) {
      listener.setCurrent(current);
      listener.setTotal(total);
      User owner = config.getUser(rec.getUser());
      AggregatedRecord newRecord = new MessagesByUserStatRecord(rec.getUser(), owner, rec);
      AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
      if (oldRecord == null) {
        putRecord(newRecord);
      } else {
        oldRecord.add(newRecord);
      }
      getTotals().add(newRecord);
      return true;
    }
  }
}