package mobi.eyeline.smsquiz.distribution.smscconsole;

import org.apache.log4j.Logger;

import java.util.concurrent.CountDownLatch;

/**
 * author: alkhal
 */
public class ConsoleConnPoolFactory {

  private static String login;
  private static String password;
  private static String host;
  private static int port;

  private static CountDownLatch initLatch = new CountDownLatch(1);

  private static final Logger logger = Logger.getLogger(ConsoleConnPoolFactory.class);

  public static void init(String login, String password, String host, int port) {
    if ((login == null) || (password == null) || (host == null)) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    ConsoleConnPoolFactory.login = login;
    ConsoleConnPoolFactory.password = password;
    ConsoleConnPoolFactory.host = host;
    ConsoleConnPoolFactory.port = port;
    initLatch.countDown();
  }

  public static ConsoleConnPool getConnectionPool(int size, long connTimeout) {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      return null;
    }
    if ((size <= 0) || (connTimeout <= 0)) {
      logger.error("Illegal arguments");
      throw new IllegalArgumentException("Illegal arguments");
    }
    return new ConsoleConnPool(size, host, port, login, password, connTimeout);
  }

}
