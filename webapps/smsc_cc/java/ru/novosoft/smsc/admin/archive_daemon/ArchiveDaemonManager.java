package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ValidationException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.File;
import java.util.Map;

/**
 * класс, управляющий настройками Archive Daemon-а
 * @author Artem Snopkov
 */
public class ArchiveDaemonManager extends ConfigFileManager<ArchiveDaemonConfig> {

  private final static ValidationHelper vh = new ValidationHelper(ArchiveDaemonManager.class);

  public ArchiveDaemonManager(File configFile, File backupDir, FileSystem fileSystem) {
    super(configFile, backupDir, fileSystem);
  }

  protected ArchiveDaemonConfig getLastAppliedConfig() {
    return super.getLastAppliedConfig();
  }

  @Override
  protected ArchiveDaemonConfig newConfigFile() {
    return new ArchiveDaemonConfig();
  }

  public int getInterval() {
    return config.getInterval();
  }

  public void setInterval(int interval) throws AdminException {
    vh.checkPositive("interval", interval);
    config.setInterval(interval);
    setChanged();
  }

  public String getViewHost() {
    return config.getViewHost();
  }

  public void setViewHost(String viewHost) throws AdminException {
    vh.checkNotEmpty("viewHost", viewHost);
    config.setViewHost(viewHost);
    setChanged();
  }

  public int getViewPort() {
    return config.getViewPort();
  }

  public void setViewPort(int viewPort) throws AdminException {
    vh.checkPort("viewPort", viewPort);
    config.setViewPort(viewPort);
    setChanged();
  }

  public int getViewTimeout() {
    return config.getViewTimeout();
  }

  public void setViewTimeout(int viewTimeout) throws AdminException {
    vh.checkPositive("viewTimeout", viewTimeout);
    config.setViewTimeout(viewTimeout);
    setChanged();
  }

  public int getQueriesMax() {
    return config.getQueriesMax();
  }

  public void setQueriesMax(int queriesMax) throws AdminException {
    vh.checkPositive("queriesMax", queriesMax);
    config.setQueriesMax(queriesMax);
    setChanged();
  }

  public int getQueriesInit() {
    return config.getQueriesInit();
  }

  public void setQueriesInit(int queriesInit) throws AdminException {
    vh.checkGreaterOrEqualsTo("queriesInit", queriesInit, 0);
    config.setQueriesInit(queriesInit);
    setChanged();
  }

  public int getTransactionsMaxSmsCount() {
    return config.getTransactionsMaxSmsCount();
  }

  public void setTransactionsMaxSmsCount(int transactionsMaxSmsCount) throws AdminException {
    vh.checkPositive("transactionsMaxSmsCount", transactionsMaxSmsCount);
    config.setTransactionsMaxSmsCount(transactionsMaxSmsCount);
    setChanged();
  }

  public int getTransactionsMaxTimeInterval() {
    return config.getTransactionsMaxTimeInterval();
  }

  public void setTransactionsMaxTimeInterval(int transactionsMaxTimeInterval) throws AdminException {
    vh.checkPositive("transactionsMaxTimeInterval", transactionsMaxTimeInterval);
    config.setTransactionsMaxTimeInterval(transactionsMaxTimeInterval);
    setChanged();
  }

  public String getLocationsBaseDestination() {
    return config.getLocationsBaseDestination();
  }

  public void setLocationsBaseDestination(String locationsBaseDestination) throws AdminException {
    vh.checkNotEmpty("locationsBaseDestination", locationsBaseDestination);
    config.setLocationsBaseDestination(locationsBaseDestination);
    setChanged();
  }

  public String getLocationsTextDestinations() {
    return config.getLocationsTextDestinations();
  }

  public void setLocationsTextDestinations(String locationsTextDestinations) throws AdminException {
    vh.checkNotEmpty("locationsTextDestinations", locationsTextDestinations);
    config.setLocationsTextDestinations(locationsTextDestinations);
    setChanged();
  }

  public Map<String, String> getLocationsSources() {
    return config.getLocationsSources();
  }

  public void setLocationsSources(Map<String, String> locationsSources) throws AdminException {
    vh.checkNoNulls("locationsSources", locationsSources);
    config.setLocationsSources(locationsSources);
    setChanged();
  }

  public int getIndexatorMaxFlushSpeed() {
    return config.getIndexatorMaxFlushSpeed();
  }

  public void setIndexatorMaxFlushSpeed(int indexatorMaxFlushSpeed) throws AdminException {
    vh.checkPositive("indexatorMaxFlushSpeed", indexatorMaxFlushSpeed);
    config.setIndexatorMaxFlushSpeed(indexatorMaxFlushSpeed);
    setChanged();
  }

  public Map<String, Integer> getIndexatorSmeAddrChunkSizes() {
    return config.getIndexatorSmeAddrChunkSizes();
  }

  public void setIndexatorSmeAddrChunkSizes(Map<String, Integer> indexatorSmeAddrChunkSizes) throws AdminException {
    vh.checkNoNulls("indexatorSmeAddrChunkSizes", indexatorSmeAddrChunkSizes);
    config.setIndexatorSmeAddrChunkSizes(indexatorSmeAddrChunkSizes);
    setChanged();
  }
}
