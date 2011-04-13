package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.utils.worker.IterativeWorker;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Date;

/**
 * User: artem
 * Date: 13.04.11
 */
class DataSourceCleaner extends IterativeWorker {

  private static final Category log = Category.getInstance("SECRET");

  private static final long DAY_DURATION_IN_MILLIS = 3600 * 1000 * 24;

  private final SecretDataSource ds;
  private final int maxAliveDays;
  private final int maxRecordsToRemoveAtOneIteration;


  DataSourceCleaner(SecretDataSource ds, int cleanInterval, int maxRecordsToRemoveAtOneIteration, int maxAliveDays) {
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
      log.debug("Removing secret records older than: " + cleanDate + ". Limit:" + maxRecordsToRemoveAtOneIteration);
    try {
      int count = ds.removeFinalizedSecretMessages(cleanDate, maxRecordsToRemoveAtOneIteration);
      if (log.isDebugEnabled())
        log.debug(count + " secret records was removed.");
    } catch (DataSourceException e) {
      log.error("Can't remove old secret records: " + e.getMessage(), e);
    }
  }

  @Override
  protected void stopCurrentWork() {
  }
}