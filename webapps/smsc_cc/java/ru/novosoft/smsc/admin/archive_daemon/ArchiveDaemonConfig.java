package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.XmlConfigHelper;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 *
 * Класс для работы с конфигурационным файлом ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */

@SuppressWarnings({"EmptyCatchBlock"})
public class ArchiveDaemonConfig {

  private final File configFile;
  private final File backupDir;
  private final FileSystem fileSystem;

  private boolean changed;

  private int interval;

  private String viewHost;
  private int viewPort;
  private int viewTimeout;

  private int queriesMax;
  private int queriesInit;

  private int transactionsMaxSmsCount;
  private int transactionsMaxTimeInterval;

  private String locationsBaseDestination;
  private String locationsTextDestinations;
  private Map<String, String> locationsSources;

  private int indexatorMaxFlushSpeed;
  private Map<String, Integer> indexatorSmeAddrChunkSizes;


  public ArchiveDaemonConfig(File baseDir, FileSystem fileSystem) throws AdminException {
    this.configFile = new File(baseDir, "conf" + File.separator + "config.xml");
    this.backupDir = new File(baseDir, "conf" + File.separator + "backup");
    this.fileSystem = fileSystem;
    reset();
  }

  public boolean isChanged() {
    return changed;
  }

  private XmlConfig loadConfig() throws XmlConfigException, AdminException {
    InputStream is = null;
    try {
      is = fileSystem.getInputStream(configFile);
      return new XmlConfig(is);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
  }

  public void reset() throws AdminException {
    try {
      XmlConfig config = loadConfig();

      interval = config.getInt("interval");

      viewHost = config.getSection("View").getString("host");
      viewPort = config.getSection("View").getInt("port");
      viewTimeout = config.getSection("View").getInt("timeout");

      queriesMax = config.getSection("Queries").getInt("max");
      queriesInit = config.getSection("Queries").getInt("init");

      transactionsMaxSmsCount = config.getSection("Transactions").getInt("maxSmsCount");
      transactionsMaxTimeInterval = config.getSection("Transactions").getInt("maxTimeInterval");

      locationsBaseDestination = config.getSection("Locations").getString("baseDestinations");
      locationsTextDestinations = config.getSection("Locations").getString("textDestinations");

      locationsSources = new HashMap<String, String>();
      XmlConfigSection sourcesSection = config.getSection("Locations").getSection("sources");
      for (XmlConfigParam sourceParam : sourcesSection.params())
        locationsSources.put(sourceParam.getName(), sourceParam.getString());

      indexatorMaxFlushSpeed = config.getSection("Indexator").getInt("maxFlushSpeed");

      indexatorSmeAddrChunkSizes = new HashMap<String, Integer>();
      XmlConfigSection chunkSizesSection = config.getSection("Indexator").getSection("smeAddrChunkSizes");
      for (XmlConfigParam chunkSizeParam : chunkSizesSection.params())
        indexatorSmeAddrChunkSizes.put(chunkSizeParam.getName(), chunkSizeParam.getInt());

    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }

    changed = false;
  }

  public void save() throws AdminException {
    try {
      XmlConfig config = loadConfig();

      config.setInt("interval", interval);

      config.getSection("View").setString("host", viewHost);
      config.getSection("View").setInt("port", viewPort);
      config.getSection("View").setInt("timeout", viewTimeout);

      config.getSection("Queries").setInt("max", queriesMax);
      config.getSection("Queries").setInt("init", queriesInit);

      config.getSection("Transactions").setInt("maxSmsCount", transactionsMaxSmsCount);
      config.getSection("Transactions").setInt("maxTimeInterval", transactionsMaxTimeInterval);

      config.getSection("Locations").setString("baseDestinations", locationsBaseDestination);
      config.getSection("Locations").setString("textDestinations", locationsTextDestinations);

      XmlConfigSection sourcesSection = config.getSection("Locations").getSection("sources");
      sourcesSection.clear();
      for (Map.Entry<String, String> location : locationsSources.entrySet())
        sourcesSection.setString(location.getKey(), location.getValue());

      config.getSection("Indexator").setInt("maxFlushSpeed", indexatorMaxFlushSpeed);

      XmlConfigSection chunkSizesSection = config.getSection("Indexator").getSection("smeAddrChunkSizes");
      chunkSizesSection.clear();
      for (Map.Entry<String, Integer> size : indexatorSmeAddrChunkSizes.entrySet())
        chunkSizesSection.setInt(size.getKey(), size.getValue());

      XmlConfigHelper.saveXmlConfig(config, configFile, backupDir, fileSystem);

    } catch (XmlConfigException e) {
      throw new AdminException("Unable to save Archive Daemon config. Cause: " + e.getMessage(), e);
    }

    changed = false;
  }

  public int getInterval() {
    return interval;
  }

  public void setInterval(int interval) {
    changed = true;
    this.interval = interval;
  }

  public String getViewHost() {
    return viewHost;
  }

  public void setViewHost(String viewHost) {
    changed = true;
    this.viewHost = viewHost;
  }

  public int getViewPort() {
    return viewPort;
  }

  public void setViewPort(int viewPort) {
    changed = true;
    this.viewPort = viewPort;
  }

  public int getViewTimeout() {
    return viewTimeout;
  }

  public void setViewTimeout(int viewTimeout) {
    changed = true;
    this.viewTimeout = viewTimeout;
  }

  public int getQueriesMax() {
    return queriesMax;
  }

  public void setQueriesMax(int queriesMax) {
    changed = true;
    this.queriesMax = queriesMax;
  }

  public int getQueriesInit() {
    return queriesInit;
  }

  public void setQueriesInit(int queriesInit) {
    changed = true;
    this.queriesInit = queriesInit;
  }

  public int getTransactionsMaxSmsCount() {
    return transactionsMaxSmsCount;
  }

  public void setTransactionsMaxSmsCount(int transactionsMaxSmsCount) {
    changed = true;
    this.transactionsMaxSmsCount = transactionsMaxSmsCount;
  }

  public int getTransactionsMaxTimeInterval() {
    return transactionsMaxTimeInterval;
  }

  public void setTransactionsMaxTimeInterval(int transactionsMaxTimeInterval) {
    changed = true;
    this.transactionsMaxTimeInterval = transactionsMaxTimeInterval;
  }

  public String getLocationsBaseDestination() {
    return locationsBaseDestination;
  }

  public void setLocationsBaseDestination(String locationsBaseDestination) {
    changed = true;
    this.locationsBaseDestination = locationsBaseDestination;
  }

  public String getLocationsTextDestinations() {
    return locationsTextDestinations;
  }

  public void setLocationsTextDestinations(String locationsTextDestinations) {
    changed = true;
    this.locationsTextDestinations = locationsTextDestinations;
  }

  public Map<String, String> getLocationsSources() {
    return new HashMap<String, String>(locationsSources);
  }

  public void setLocationsSources(Map<String, String> locationsSources) {
    changed = true;
    this.locationsSources = new HashMap<String, String>(locationsSources);
  }

  public int getIndexatorMaxFlushSpeed() {
    return indexatorMaxFlushSpeed;
  }

  public void setIndexatorMaxFlushSpeed(int indexatorMaxFlushSpeed) {
    changed = true;
    this.indexatorMaxFlushSpeed = indexatorMaxFlushSpeed;
  }

  public Map<String, Integer> getIndexatorSmeAddrChunkSizes() {
    return new HashMap<String, Integer>(indexatorSmeAddrChunkSizes);
  }

  public void setIndexatorSmeAddrChunkSizes(Map<String, Integer> indexatorSmeAddrChunkSizes) {
    changed = true;
    this.indexatorSmeAddrChunkSizes = new HashMap<String, Integer>(indexatorSmeAddrChunkSizes);
  }
}
