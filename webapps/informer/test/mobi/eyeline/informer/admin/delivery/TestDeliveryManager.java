package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatProvider;

/**
 * @author Aleksandr Khalitov
 */
public class TestDeliveryManager extends DeliveryManager{

  public TestDeliveryManager(DeliveryStatProvider provider) {
    super(provider);
  }

  private TestDcpConnection connection = new TestDcpConnection();

  @Override
  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return connection;
  }

  void forceModifyDeliveries() throws AdminException {
    connection.modifyAll();
  }

  public void forceActivatePlannedDeliveries() throws AdminException {
    connection.forceActivate();
  }
  
}
