package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.users.User;
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
    start();     
  }


  @Override
  public void loadRecords(final Configuration config, final Locale locale) throws AdminException {
    DeliveryStatFilter filterCopy = new DeliveryStatFilter(getFilter());

    config.statistics(filterCopy, new DeliveryStatVisitor() {

      public boolean visit(DeliveryStatRecord rec, int total, int current) {
        setCurrentAndTotal(current, total);
        User owner = config.getUser(rec.getUser());
        AggregatedRecord newRecord = new MessagesByUserStatRecord(rec.getUser(), owner, rec);
        AggregatedRecord oldRecord = getRecord(newRecord.getAggregationKey());
        if (oldRecord == null) {
          putRecord(newRecord);
        } else {
          oldRecord.add(newRecord);
        }
        getTotals().add(newRecord);
        return !isCancelled();
      }
    });
  }


}