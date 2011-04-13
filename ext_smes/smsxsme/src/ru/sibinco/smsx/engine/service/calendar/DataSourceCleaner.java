package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.utils.worker.IterativeWorker;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Date;

/**
 * User: artem
 * Date: 13.04.11
 */
class DataSourceCleaner extends IterativeWorker {

  private static final Category log = Category.getInstance("CALENDAR");

  private static final long DAY_DURATION_IN_MILLIS = 3600 * 1000 * 24;

  private final CalendarDataSource ds;
  private final int maxAliveDays;
  private final int maxRecordsToRemoveAtOneIteration;


  DataSourceCleaner(CalendarDataSource ds, int cleanInterval, int maxRecordsToRemoveAtOneIteration, int maxAliveDays) {
    super(log);
    setDelayBetweenIterations(cleanInterval);
    this.maxRecordsToRemoveAtOneIteration = maxRecordsToRemoveAtOneIteration;
    this.ds = ds;
    this.maxAliveDays = maxAliveDays;
  }

  @Override
  protected void iterativeWork() {
    Date cleanDate = new Date(System.currentTimeMillis() - DAY_DURATION_IN_MILLIS * maxAliveDays);
    if (log.isDebugEnabled())
      log.debug("Removing calendar records older than: " + cleanDate + ". Limit:" + maxRecordsToRemoveAtOneIteration);
    try {
      int count = ds.removeCalendarMessages(cleanDate, maxRecordsToRemoveAtOneIteration);
      if (log.isDebugEnabled())
        log.debug(count + " calendar records was removed.");
    } catch (DataSourceException e) {
      log.error("Can't remove old calendar records: " + e.getMessage(), e);
    }
  }

  @Override
  protected void stopCurrentWork() {
  }
}
