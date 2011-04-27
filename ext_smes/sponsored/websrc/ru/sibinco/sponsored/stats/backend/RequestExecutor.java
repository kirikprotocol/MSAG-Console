package ru.sibinco.sponsored.stats.backend;

import ru.sibinco.sponsored.stats.backend.datasource.DeliveryStat;
import ru.sibinco.sponsored.stats.backend.datasource.FileDeliveryStatDataSource;
import ru.sibinco.sponsored.stats.backend.datasource.ResultSet;
import ru.sibinco.sponsored.stats.backend.datasource.DeliveryStatsQuery;

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


  void execute(final SponsoredRequest request) throws StatisticsException {
    ResultSet rs = null;
    Progress p = new Progress() {
      public void setProgress(int progress) {
        request.setProgress(progress); //todo Здесь надо progress умножать на 3/4. Иначе выходит, что на выходе их метода aggregateDeliveryStats прогресс уже 100%
                                       //todo и то, что потом еще формируется файл с отчетом - не учитывается.
      }
    };
    try{
      rs = deliveryStatDataSource.aggregateDeliveryStats(request.getFrom(), request.getTill(), new DeliveryStatsQuery() {
        public boolean isAllowed(DeliveryStat stat) {
          return true;
        }
      }, p);
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
