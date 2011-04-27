package ru.sibinco.smsx.stats.backend;

import ru.sibinco.smsx.stats.backend.datasource.*;

import java.util.Iterator;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
class RequestExecutor {

  private final SmsxStatisticsManager statisticsManager;

  private final ResultsManager resultsManager;

  RequestExecutor(SmsxStatisticsManager statisticsManager, ResultsManager resultsManager) {
    this.statisticsManager = statisticsManager;
    this.resultsManager = resultsManager;
  }


  void execute(final SmsxRequest request, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    Set reports = request.getReportTypesFilter();
    final int totalReports = reports == null ? 4 : reports.size();
    final int[] countReport = new int[]{0};
    final int[] previousProgress = new int[]{0};
    class ProgressListenerImpl implements ProgressListener {
      public void setProgress(int progress) {
        request.setProgress(previousProgress[0]+ (progress*countReport[0]/totalReports));
      }
    }
    ProgressListener p = new ProgressListenerImpl();
    if(reports == null || reports.contains(SmsxRequest.ReportType.WEB_DAILY)) {
      processWebDaily(request, p, shutdownIndicator);
      previousProgress[0] = request.getProgress();
    }
    if(reports == null || reports.contains(SmsxRequest.ReportType.SMSX_USERS)) {
      processSmsxUsers(request, p, shutdownIndicator);
      previousProgress[0] = request.getProgress();
    }
    if(reports == null || reports.contains(SmsxRequest.ReportType.WEB_REGIONS)) {
      processWebRegions(request, p, shutdownIndicator);
      previousProgress[0] = request.getProgress();
    }
    if(reports == null || reports.contains(SmsxRequest.ReportType.TRAFFIC)) {
      processTraffic(request, p, shutdownIndicator);
    }
  }

  private void processWebDaily(SmsxRequest request, ProgressListener p, ShutdownIndicator shutdownIndicator) throws StatisticsException{
    ResultsManager.WebDailyResults res = null;
    try{
      res = resultsManager.createWebDailyResult(request.getId());
      Iterator i = statisticsManager.webSmsDaily(request.getFrom(), request.getTill(), p, shutdownIndicator).iterator();
      while(i.hasNext()) {
        if(shutdownIndicator.isShutdown()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        res.write((WebDaily)i.next());
      }
    }finally {
      if(res != null) {
        res.close();
      }
    }
  }

  private void processSmsxUsers(SmsxRequest request, ProgressListener p, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    ResultsManager.SmsxUserResults res = null;
    try{
      res = resultsManager.createSmsxUsersResult(request.getId());
      Iterator i = statisticsManager.smsxUsers(request.getFrom(), request.getTill(), p, request.getServiceIdFilter(), shutdownIndicator).iterator();
      while(i.hasNext()) {
        if(shutdownIndicator.isShutdown()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        res.write((SmsxUsers) i.next());
      }
    }finally {
      if(res != null) {
        res.close();
      }
    }
  }

  private void processTraffic(SmsxRequest request, ProgressListener p, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    ResultsManager.TrafficResults res = null;
    try{
      res = resultsManager.createTrafficResult(request.getId());
      Iterator i = statisticsManager.traffic(request.getFrom(), request.getTill(), p, request.getServiceIdFilter(), shutdownIndicator).iterator();
      while(i.hasNext()) {
        if(shutdownIndicator.isShutdown()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        res.write((Traffic) i.next());
      }
    }finally {
      if(res != null) {
        res.close();
      }
    }
  }

  private void processWebRegions(SmsxRequest request, ProgressListener p, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    ResultsManager.WebRegionsResults res = null;
    try{
      res = resultsManager.createWebRegionsResult(request.getId());
      Iterator i = statisticsManager.webSmsRegions(request.getFrom(), request.getTill(), p, shutdownIndicator).iterator();
      while(i.hasNext()) {
        if(shutdownIndicator.isShutdown()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        res.write((WebRegion) i.next());
      }
    }finally {
      if(res != null) {
        res.close();
      }
    }
  }


}
