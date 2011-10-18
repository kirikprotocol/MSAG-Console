package ru.novosoft.smsc.admin.perfmon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

import java.io.IOException;

/**
 * author: Aleksandr Khalitov
 */
public class PerfMonitorManager {

  private static final Logger logger = Logger.getLogger(PerfMonitorManager.class);

  private PerfServer[] perfServer;

  private final PerfMonitorContext context;

  public PerfMonitorManager(PerfMonitorContext context) throws AdminException{
    this.context = context;
    initPerfServers();
  }

  protected PerfServer createServer(int clientPort, InetAddress smscAddress, boolean support64Bit) throws IOException {
    return new PerfServer(clientPort, smscAddress, support64Bit);
  }

  private void initPerfServers() throws AdminException{
    int count = context.getPerfMonitorCount();
    perfServer = new PerfServer[count];
    try{
      for(int i=0; i<count;i++) {
        InetAddress address = context.getPerfMonitorAddress(i);
        perfServer[i] = createServer(context.getAppletPort(i), address, context.isSupport64Bit());
        perfServer[i].start();
      }
    }catch (IOException e) {
      logger.error(e, e);
      throw new PerfMonitorException("io_error", e);
    }
  }

  public int getAppletPort(int instance) throws AdminException {
    try{
      return context.getAppletPort(instance);
    }catch (IndexOutOfBoundsException e) {
      return -1;
    }
  }

  public void updatePerfServers() throws AdminException {
    shutdown();
    initPerfServers();
  }

  public void shutdown() {
    if(perfServer != null) {
      for(PerfServer s : perfServer) {
        try{
          s.shutdown();
        }catch (Exception ignored){}
      }
    }
  }

}
