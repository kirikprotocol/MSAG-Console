package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.util.Collection;
import java.util.Date;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxStatisticsManager {
  
  private File artefactsDir;

  public SmsxStatisticsManager(File artefactsDir) throws StatisticsException {
     if(artefactsDir == null || !artefactsDir.exists()) {
      throw new StatisticsException("Directory doesn't exist: "+(artefactsDir == null ? null : artefactsDir.getAbsolutePath()));
    }
    this.artefactsDir = artefactsDir;
  }

  public Collection smsxUsers(final Date from, final Date till, final ProgressListener _progressListener, Set serviceId,
                              ShutdownIndicator shutdownIndicator) throws StatisticsException {
    SmsxFileProcessor smsxUsers = new SmsxFileProcessor(artefactsDir, from, till, _progressListener, serviceId);
    return smsxUsers.process(shutdownIndicator);
  }

  public Collection webSmsRegions(final Date from, final Date till, final ProgressListener _progressListener,
                                  ShutdownIndicator shutdownIndicator) throws StatisticsException{
    WebRegionsStatsProcessor webSmsRegions = new WebRegionsStatsProcessor(artefactsDir, from, till, _progressListener);
    return webSmsRegions.process(shutdownIndicator);
  }

  public Collection webSmsDaily(final Date from, final Date till, final ProgressListener _progressListener,
                                ShutdownIndicator shutdownIndicator) throws StatisticsException{
    DailyFileProcessor webSmsDaily = new DailyFileProcessor(artefactsDir, from, till, _progressListener);
    return webSmsDaily.process(shutdownIndicator);
  }

  public Collection traffic(final Date from, final Date till, final ProgressListener _progressListener,
                            Set serviceId, ShutdownIndicator shutdownIndicator) throws StatisticsException{
    TrafficFileProcessor traffic = new TrafficFileProcessor(artefactsDir, from, till, _progressListener, serviceId);
    return traffic.process(shutdownIndicator);
  }

}
