package ru.sibinco.smsx.stats.backend;

import ru.sibinco.smsx.stats.backend.datasource.*;

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


  void execute(final SmsxRequest request) throws StatisticsException {
    Set reports = request.getReportTypesFilter();
    final int totalReports = reports == null ? 4 : reports.size();
    final int[] countReport = new int[]{0};
    final int[] previousProgress = new int[]{0};
    class ProgressImpl extends Progress{
      public void setProgress(int progress) {
        request.setProgress(previousProgress[0]+ (progress*countReport[0]/totalReports));
      }
    }
    Progress p = new ProgressImpl();
    if(reports == null || reports.contains(SmsxRequest.ReportType.WEB_DAILY)) {
      processWebDaily(request, p);
      previousProgress[0] = request.getProgress();
    }
    if(reports == null || reports.contains(SmsxRequest.ReportType.SMSX_USERS)) {
      processSmsxUsers(request, p);
      previousProgress[0] = request.getProgress();
    }
    if(reports == null || reports.contains(SmsxRequest.ReportType.WEB_REGIONS)) {
      processWebRegions(request, p);
      previousProgress[0] = request.getProgress();
    }
    if(reports == null || reports.contains(SmsxRequest.ReportType.TRAFFIC)) {
      processTraffic(request, p);
    }
  }

  private void processWebDaily(SmsxRequest request, Progress p) throws StatisticsException{
    final ResultsManager.WebDailyResults[] res = new ResultsManager.WebDailyResults[]{null};
    try{
      res[0] = resultsManager.createWebDailyResult(request.getId());
      statisticsManager.webSmsDaily(new Visitor() {
        public boolean visit(Object o) throws StatisticsException {
          res[0].write((WebDaily)o);
          return true;
        }
      }, request.getFrom(), request.getTill(), p);
    }finally {
      if(res[0] != null) {
        res[0].close();
      }
    }
  }

  private void processSmsxUsers(SmsxRequest request, Progress p) throws StatisticsException {
    final ResultsManager.SmsxUserResults[] res = new ResultsManager.SmsxUserResults[]{null};
    try{
      res[0] = resultsManager.createSmsxUsersResult(request.getId());
      statisticsManager.smsxUsers(new Visitor() {
        public boolean visit(Object o) throws StatisticsException {
          res[0].write((SmsxUsers) o);
          return true;
        }
      }, request.getFrom(), request.getTill(), p, request.getServiceIdFilter());
    }finally {
      if(res[0] != null) {
        res[0].close();
      }
    }
  }

  private void processTraffic(SmsxRequest request, Progress p) throws StatisticsException {
    final ResultsManager.TrafficResults[] res = new ResultsManager.TrafficResults[]{null};
    try{
      res[0] = resultsManager.createTrafficResult(request.getId());
      statisticsManager.traffic(new Visitor() {
        public boolean visit(Object o) throws StatisticsException {
          res[0].write((Traffic) o);
          return true;
        }
      }, request.getFrom(), request.getTill(), p, request.getServiceIdFilter());
    }finally {
      if(res[0] != null) {
        res[0].close();
      }
    }
  }

  private void processWebRegions(SmsxRequest request, Progress p) throws StatisticsException {
    final ResultsManager.WebRegionsResults[] res = new ResultsManager.WebRegionsResults[]{null};
    try{
      res[0] = resultsManager.createWebRegionsResult(request.getId());
      statisticsManager.webSmsRegions(new Visitor() {
        public boolean visit(Object o) throws StatisticsException {
          res[0].write((WebRegion) o);
          return true;
        }
      }, request.getFrom(), request.getTill(), p);
    }finally {
      if(res[0] != null) {
        res[0].close();
      }
    }
  }


}
