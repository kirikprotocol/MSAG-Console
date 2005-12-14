package ru.novosoft.smsc.mcisme.backend;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.sibinco.util.conpool.ConnectionPool;

import javax.sql.DataSource;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;


/**
 * Created by IntelliJ IDEA. User: makar Date: 02.04.2004 Time: 16:19:19 To change this template use Options | File Templates.
 */
public class MCISmeContext {
  private static final Map instances = new HashMap();
  private Category logger = Category.getInstance(this.getClass());

  public static synchronized MCISmeContext getInstance(final SMSCAppContext appContext, final String smeId)
          throws AdminException, ParserConfigurationException, IOException, SAXException, Config.WrongParamTypeException, Config.ParamNotFoundException {
    MCISmeContext instance = (MCISmeContext) instances.get(smeId);
    if (null == instance) {
      instance = new MCISmeContext(appContext, smeId);
      instances.put(smeId, instance);
    }
    return instance;
  }

  private final SMSCAppContext appContext;
  private Config config = null;
  private MCISme mciSme = null;

  private boolean changedOptions = false;
  private boolean changedDrivers = false;
  private boolean changedTemplates = false;
  private boolean changedRules = false;
  private boolean changedOffsets = false;

  private DataSource dataSource = null;
  private String smeId = "MCISme";


  private MCISmeContext(final SMSCAppContext appContext, final String smeId)
          throws AdminException, ParserConfigurationException, SAXException, IOException, Config.WrongParamTypeException, Config.ParamNotFoundException {
    this.smeId = smeId;
    this.appContext = appContext;
    resetConfig();
    this.mciSme = new MCISme(appContext.getHostsManager().getServiceInfo(this.smeId),
            config.getString("MCISme.Admin.host"), config.getInt("MCISme.Admin.port"));
  }

  public Config loadCurrentConfig()
          throws AdminException, IOException, SAXException, ParserConfigurationException {
    return new Config(new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(),
            "conf" + File.separatorChar + "config.xml"));
  }

  public void resetConfig()
          throws AdminException, SAXException, ParserConfigurationException, IOException {
    final Config newConfig = loadCurrentConfig();
    reloadDataSource(config, newConfig);
    config = newConfig;
  }

  public Config getConfig() {
    return config;
  }

  public void reloadDataSource(final Config oldConfig, final Config newConfig) {
    try {
      if (null == oldConfig
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.jdbc.source")
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.jdbc.driver")
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.dbUserName")
              || !Config.isParamEquals(oldConfig, newConfig, "MCISme.DataSource.dbUserPassword")) {
        dataSource = null;
        final Properties properties = new Properties();
        properties.setProperty("jdbc.source", newConfig.getString("MCISme.DataSource.jdbc.source"));
        properties.setProperty("jdbc.driver", newConfig.getString("MCISme.DataSource.jdbc.driver"));
        properties.setProperty("jdbc.user", newConfig.getString("MCISme.DataSource.dbUserName"));
        properties.setProperty("jdbc.pass", newConfig.getString("MCISme.DataSource.dbUserPassword"));
        properties.setProperty("jdbc.min.connections", "0");
        properties.setProperty("jdbc.max.idle.time", "240");
        properties.setProperty("jdbc.pool.name", "mcisme");
        dataSource = new ConnectionPool(properties);
      }
    } catch (Throwable e) {
      logger.error("Could not init datasource", e);
    }
  }

  public MCISme getMCISme() {
    return mciSme;
  }

  public boolean isChangedOptions() {
    return changedOptions;
  }

  public void setChangedOptions(final boolean changedOptions) {
    this.changedOptions = changedOptions;
  }

  public boolean isChangedDrivers() {
    return changedDrivers;
  }

  public void setChangedDrivers(final boolean changedDrivers) {
    this.changedDrivers = changedDrivers;
  }

  public boolean isChangedTemplates() {
    return changedTemplates;
  }

  public void setChangedTemplates(final boolean changedTemplates) {
    this.changedTemplates = changedTemplates;
  }

  public boolean isChangedRules() {
    return changedRules;
  }

  public void setChangedRules(boolean changedRules) {
    this.changedRules = changedRules;
  }

  public boolean isChangedOffsets() {
    return changedOffsets;
  }

  public void setChangedOffsets(boolean changedOffsets) {
    this.changedOffsets = changedOffsets;
  }

  public DataSource getDataSource() {
    return dataSource;
  }
}

