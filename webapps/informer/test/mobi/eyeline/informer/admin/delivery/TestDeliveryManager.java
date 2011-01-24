package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatProvider;
import mobi.eyeline.informer.admin.delivery.stat.UserStatProvider;

/**
 * @author Aleksandr Khalitov
 */
public class TestDeliveryManager extends DeliveryManager{

  public TestDeliveryManager(DeliveryStatProvider provider, UserStatProvider userStatsProvider) {
    super(provider, userStatsProvider);
  }

  private TestDcpConnection connection = new TestDcpConnection();

  @Override
  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return connection;
  }

  public void forceModifyDeliveries() throws AdminException {
    connection.modifyAll();
  }

  public void forceActivatePlannedDeliveries() throws AdminException {
    connection.forceActivate();
  }
  
}
