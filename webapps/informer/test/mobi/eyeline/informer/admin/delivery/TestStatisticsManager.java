package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatProvider;
import mobi.eyeline.informer.admin.delivery.stat.UserStatProvider;

/**
 * @author Aleksandr Khalitov
 */
public class TestStatisticsManager extends StatisticsManager{

  public TestStatisticsManager(DeliveryStatProvider statsProvider, UserStatProvider userStatsProvider) {
    super(statsProvider, userStatsProvider);
  }

}
