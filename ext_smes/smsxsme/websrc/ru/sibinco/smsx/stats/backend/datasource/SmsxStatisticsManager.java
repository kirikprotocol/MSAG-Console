package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.File;
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

  public void smsxUsers(Visitor v, final Date from, final Date till, final Progress _progress, Set serviceId) throws StatisticsException {
    FileStatsProcessor smsxUsers = new SmsxFileProcessor(artefactsDir, from, till, _progress, serviceId);
    smsxUsers.process(v);
  }

  public void webSmsRegions(Visitor v, final Date from, final Date till, final Progress _progress) throws StatisticsException{
    FileStatsProcessor webSmsRegions = new WebRegionsStatsProcessor(artefactsDir, from, till, _progress);
    webSmsRegions.process(v);
  }


  public void webSmsDaily(Visitor v, final Date from, final Date till, final Progress _progress) throws StatisticsException{
    FileStatsProcessor webSmsDaily = new DailyFileProcessor(artefactsDir, from, till, _progress);
    webSmsDaily.process(v);
  }


  public void traffic(Visitor v, final Date from, final Date till, final Progress _progress, Set serviceId) throws StatisticsException{
    FileStatsProcessor traffic = new TrafficFileProcessor(artefactsDir, from, till, _progress, serviceId);
    traffic.process(v);
  }


}
