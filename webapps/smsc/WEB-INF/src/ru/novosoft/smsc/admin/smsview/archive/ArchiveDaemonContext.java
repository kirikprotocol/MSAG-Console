package ru.novosoft.smsc.admin.smsview.archive;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;

import java.io.File;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.02.2004
 * Time: 12:50:31
 * To change this template use Options | File Templates.
 */
public class ArchiveDaemonContext
{
  private static ArchiveDaemonContext instance = null;

  public static synchronized ArchiveDaemonContext getInstance(SMSCAppContext appContext)  throws AdminException
  {
    if (instance == null)
      instance = new ArchiveDaemonContext(appContext);
    return instance;
  }

  private final SMSCAppContext appContext;
  private Config config = null;
  private String host;
  private short  port;

  private ArchiveDaemonContext(SMSCAppContext appContext) throws AdminException
  {
    this.appContext = appContext;
    resetConfig(true);
  }

  public void resetConfig(boolean load) throws AdminException
  {
    try {
      if (load) {
        config = new Config(new File(appContext.getHostsManager().getServiceInfo(Constants.ARCHIVE_DAEMON_SVC_ID).getServiceFolder(),
                                     "conf" + File.separatorChar + "daemon.xml"));
      }
      host = config.getString("ArchiveDaemon.View.host");
      port = (short)config.getInt("ArchiveDaemon.View.port");
    } catch (Throwable t) {
      t.printStackTrace();
      throw new AdminException(t.getMessage());
    }
  }
  public void saveConfig(Config config) throws AdminException
  {
    try {
      config.save("ISO-8859-1");
    } catch (Throwable t) {
      throw new AdminException("Couldn't store SMSC config: " + t.getMessage());
    }
    this.config = config;
    resetConfig(false);
  }

  public Config getConfig() {
    return config;
  }
  public String getHost() {
    return host;
  }
  public short getPort() {
    return port;
  }
}
