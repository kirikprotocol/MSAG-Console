package ru.sibinco.sponsored.stats.backend;

import ru.sibinco.sponsored.stats.backend.datasource.*;

/**
 * @author Aleksandr Khalitov
 */
class RequestExecutor {

  private final FileDeliveryStatDataSource deliveryStatDataSource;

  private final ResultsManager resultsManager;

  RequestExecutor(FileDeliveryStatDataSource deliveryStatDataSource, ResultsManager resultsManager) {
    this.deliveryStatDataSource = deliveryStatDataSource;
    this.resultsManager = resultsManager;
  }


  void execute(final SponsoredRequest request, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    ResultSet rs = null;
    ProgressListener p = new ProgressListener() {
      public void setProgress(int progress) {
        request.setProgress(3*progress/4);
      }
    };
    try{
      rs = deliveryStatDataSource.aggregateDeliveryStats(request.getId(), request.getFrom(), request.getTill(), new DeliveryStatsQuery() {
        public boolean isAllowed(DeliveryStat stat) {
          return true;
        }
      }, p, shutdownIndicator);
      if(rs == null) {
        p.setProgress(100);
        return;
      }
      ResultsManager.SponsoredResults results = null;
      try{
        results = resultsManager.createSponsoredResult(request.getId());
        while(rs.next()) {
          DeliveryStat ds = (DeliveryStat)rs.get();
          results.write(ds.getSubscriberAddress(), ds.getDelivered()* 100000 * request.getCost() / 100000);
        }
      }finally {
        if(results != null) {
          results.close();
        }
        p.setProgress(100);
      }
    }finally {
      if(rs != null) {
        try{
          rs.close();
        }catch (Exception ignored){}
      }
    }
  }


}
