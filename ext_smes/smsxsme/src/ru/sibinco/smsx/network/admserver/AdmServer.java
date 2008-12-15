package ru.sibinco.smsx.network.admserver;

import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.admin.server.CommandServer;
import ru.sibinco.smsx.InitializationException;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class AdmServer {

  private static CommandServer instance;
  private static final CountDownLatch initLatch = new CountDownLatch(1);

  public static void init(XmlConfig config)  {
    if (instance == null) {
      try {
        int port = config.getSection("adminserver").getInt("port");
        int poolSize = config.getSection("adminserver").getInt("poolSize");
        int timeout = config.getSection("adminserver").getInt("timeout");

        instance = new CommandServer(port, 1, poolSize, 60000, timeout);
        instance.start();

        initLatch.countDown();
      } catch (Exception e) {
        throw new InitializationException(e);
      }
    }
  }

  public static CommandServer getInstance() {
    try {
      initLatch.await();
      return instance;
    } catch (InterruptedException e) {
      return null;
    }
  }

  public static void shutdown() {
    if (instance != null)
      instance.shutdown();
  }
}
