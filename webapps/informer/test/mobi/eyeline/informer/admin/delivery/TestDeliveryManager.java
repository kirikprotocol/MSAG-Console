package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class TestDeliveryManager extends DeliveryManager{

  public TestDeliveryManager() {
    super();
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
