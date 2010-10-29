package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

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
  
}
