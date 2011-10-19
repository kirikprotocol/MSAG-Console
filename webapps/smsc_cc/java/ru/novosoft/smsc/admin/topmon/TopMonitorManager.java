package ru.novosoft.smsc.admin.topmon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

import java.io.IOException;

/**
 * author: Aleksandr Khalitov
 */
public class TopMonitorManager {

  private static final Logger logger = Logger.getLogger(TopMonitorManager.class);

  private TopServer[] perfServer;

  private final TopMonitorContext context;

  public TopMonitorManager(TopMonitorContext context) throws AdminException {
    this.context = context;
    initPerfServers();
  }

  protected TopServer createServer(int clientPort, InetAddress smscAddress) throws IOException {
    return new TopServer(clientPort, smscAddress);
  }

  private void initPerfServers() throws AdminException {
    int count = context.getTopMonitorCount();
    perfServer = new TopServer[count];
    try {
      for (int i = 0; i < count; i++) {
        InetAddress address = context.getTopMonitorAddress(i);
        perfServer[i] = createServer(context.getAppletPort(i), address);
        perfServer[i].start();
      }
    } catch (IOException e) {
      logger.error(e, e);
      throw new TopMonitorException("io_error", e);
    }
  }

  public int getAppletPort(int instance) throws AdminException {
    try {
      return context.getAppletPort(instance);
    } catch (IndexOutOfBoundsException e) {
      return -1;
    }
  }

  public void updateTopServers() throws AdminException {
    shutdown();
    initPerfServers();
  }

  public void shutdown() {
    if (perfServer != null) {
      for (TopServer s : perfServer) {
        try {
          s.shutdown();
        } catch (Exception ignored) {
        }
      }
    }
  }
}
