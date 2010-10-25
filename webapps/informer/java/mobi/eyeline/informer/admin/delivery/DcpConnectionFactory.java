package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.DcpConnectionImpl;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
class DcpConnectionFactory {

  private final Map<User, DcpConnection> pool = new HashMap<User, DcpConnection>();

  private final String host;

  private final int port;

  DcpConnectionFactory(String host, int port) {
    this.host = host;
    this.port = port;
  }

  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return new DcpConnectionImpl(host, port, login, password);
  }

  synchronized DcpConnection getDeliveryConnection(String login, String password) throws AdminException {
    User u = new User(login, password);
    DcpConnection connection = pool.get(u);
    if (connection == null) {
      connection = createConnection(host, port, login, password);
      pool.put(u, connection);
    }
    return connection;
  }

  synchronized void shutdown() {
    for (DcpConnection conn : pool.values()) {
      try {
        conn.close();
      } catch (Exception ignored) {
      }
    }
  }

  private static class User {
    private final String login;
    private final String password;

    private User(String login, String password) {
      this.login = login;
      this.password = password;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;
      User user = (User) o;
      return !(login != null ? !login.equals(user.login) : user.login != null) &&
          !(password != null ? !password.equals(user.password) : user.password != null);
    }

    @Override
    public int hashCode() {
      int result = login != null ? login.hashCode() : 0;
      result = 31 * result + (password != null ? password.hashCode() : 0);
      return result;
    }
  }
}
