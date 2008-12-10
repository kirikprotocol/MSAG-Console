package mobi.eyeline.smsquiz.distribution.smscconsole;

import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
public class ConsoleConnPoolFactory {

  private static String login;
  private static String password;
  private static String host;
  private static int port;

  private static boolean init = false;

  private static final Logger logger = Logger.getLogger(ConsoleConnPoolFactory.class);
  
  public static synchronized void init(String login, String password, String host, int port) {
    if((login == null)||(password == null) ||(host == null)) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    ConsoleConnPoolFactory.login = login;
    ConsoleConnPoolFactory.password = password;
    ConsoleConnPoolFactory.host = host;
    ConsoleConnPoolFactory.port = port;
    init = true;
  }

  public static synchronized ConsoleConnPool getConnectionPool(int size, long connTimeout) {
    if(!init) {
      logger.error("Please, init this class first");
      throw new IllegalStateException("Please, init this class first");
    }
    if((size<=0)||(connTimeout<=0)) {
      logger.error("Illegal arguments");
      throw new IllegalArgumentException("Illegal arguments");
    }
    return new ConsoleConnPool(size, host, port, login, password, connTimeout);
  }

}
