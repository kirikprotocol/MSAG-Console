package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class TestDcpConnectionFactory extends DcpConnectionFactory {

  private DcpConnection connection;

  public TestDcpConnectionFactory() {
    super(null, 0);
    connection = new TestDcpConnection();
  }

  @Override
  protected synchronized DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return connection;
  }
  
}
