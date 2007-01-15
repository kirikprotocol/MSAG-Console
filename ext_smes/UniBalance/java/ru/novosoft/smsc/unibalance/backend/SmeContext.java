package ru.novosoft.smsc.unibalance.backend;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.Properties;

/**
 * User: artem
 * Date: 12.01.2007
 */
public class SmeContext implements SMEAppContext{
  public static final String SME_ID = "unibalance";

  private Properties config = null;
  private final Object configMonitor = new Object();

  private static SmeContext instance = null;

  public static SmeContext getInstance(SMSCAppContext appContext) throws AdminException, IOException, SAXException, ParserConfigurationException {
    return instance == null ? instance = new SmeContext(appContext) : instance;
  }

  private final SMSCAppContext appContext;

  public SmeContext(SMSCAppContext appContext) throws IOException, ParserConfigurationException, SAXException, AdminException {
    this.appContext = appContext;
    appContext.registerSMEContext(this);
    resetConfig();
  }

  public void resetConfig() throws AdminException, IOException, ParserConfigurationException, SAXException {
    synchronized(configMonitor) {
      config = loadCurrentConfig();
    }
  }

  public Properties getConfig() {
    synchronized(configMonitor) {
      return config;
    }
  }

  private File getConfigFile() throws AdminException {
    return new File(appContext.getHostsManager().getServiceInfo(SME_ID).getServiceFolder(), "conf" + File.separatorChar + "response.pattern.properties");
  }

  public Properties loadCurrentConfig() throws AdminException, IOException, SAXException, ParserConfigurationException {
    final Properties config = new Properties();
    config.load(new FileInputStream(getConfigFile()));
    return config;
  }

  public void storeConfig(Properties config) throws AdminException, IOException {
    synchronized(configMonitor) {
      config.store(new FileOutputStream(getConfigFile()), null);
      this.config = config;
    }
  }

  public void shutdown() {
  }
}
