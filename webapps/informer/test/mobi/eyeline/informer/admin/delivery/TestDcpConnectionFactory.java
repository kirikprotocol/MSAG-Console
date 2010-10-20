package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class TestDcpConnectionFactory extends DcpConnectionFactory {

  public TestDcpConnectionFactory() {
    super(null, 0);
  }

  @Override
  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return super.createConnection(host, port, login, password);
  }


//  private static class TestDcpConnection implements DcpConnection {
//
//    public void connect(String login, String password) throws AdminException {}
//
//    public long[] addMessages() throws AdminException {
//      return new long[0];
//    }
//  }
}
