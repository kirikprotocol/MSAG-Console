package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonManager extends ConfigFileManager<ArchiveDaemonConfig> {

  public ArchiveDaemonManager(File configFile, File backupDir, FileSystem fileSystem) throws AdminException {
    super(configFile, backupDir, fileSystem);
    reset();
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

  public void setInterval(int interval) {
    config.setInterval(interval);
    changed = true;
  }

  public String getViewHost() {
    return config.getViewHost();
  }

  public void setViewHost(String viewHost) {
    config.setViewHost(viewHost);
    changed = true;
  }

  public int getViewPort() {
    return config.getViewPort();
  }

  public void setViewPort(int viewPort) {
    config.setViewPort(viewPort);
    changed = true;
  }

  public int getViewTimeout() {
    return config.getViewTimeout();
  }

  public void setViewTimeout(int viewTimeout) {
    config.setViewTimeout(viewTimeout);
    changed = true;
  }

  public int getQueriesMax() {
    return config.getQueriesMax();
  }

  public void setQueriesMax(int queriesMax) {
    config.setQueriesMax(queriesMax);
    changed = true;
  }

  public int getQueriesInit() {
    return config.getQueriesInit();
  }

  public void setQueriesInit(int queriesInit) {
    config.setQueriesInit(queriesInit);
    changed = true;
  }

  public int getTransactionsMaxSmsCount() {
    return config.getTransactionsMaxSmsCount();
  }

  public void setTransactionsMaxSmsCount(int transactionsMaxSmsCount) {
    config.setTransactionsMaxSmsCount(transactionsMaxSmsCount);
    changed = true;
  }

  public int getTransactionsMaxTimeInterval() {
    return config.getTransactionsMaxTimeInterval();
  }

  public void setTransactionsMaxTimeInterval(int transactionsMaxTimeInterval) {
    config.setTransactionsMaxTimeInterval(transactionsMaxTimeInterval);
    changed = true;
  }

  public String getLocationsBaseDestination() {
    return config.getLocationsBaseDestination();
  }

  public void setLocationsBaseDestination(String locationsBaseDestination) {
    config.setLocationsBaseDestination(locationsBaseDestination);
    changed = true;
  }

  public String getLocationsTextDestinations() {
    return config.getLocationsTextDestinations();
  }

  public void setLocationsTextDestinations(String locationsTextDestinations) {
    config.setLocationsTextDestinations(locationsTextDestinations);
    changed = true;
  }

  public Map<String, String> getLocationsSources() {
    return config.getLocationsSources();
  }

  public void setLocationsSources(Map<String, String> locationsSources) {
    config.setLocationsSources(locationsSources);
    changed = true;
  }

  public int getIndexatorMaxFlushSpeed() {
    return config.getIndexatorMaxFlushSpeed();
  }

  public void setIndexatorMaxFlushSpeed(int indexatorMaxFlushSpeed) {
    config.setIndexatorMaxFlushSpeed(indexatorMaxFlushSpeed);
    changed = true;
  }

  public Map<String, Integer> getIndexatorSmeAddrChunkSizes() {
    return config.getIndexatorSmeAddrChunkSizes();
  }

  public void setIndexatorSmeAddrChunkSizes(Map<String, Integer> indexatorSmeAddrChunkSizes) {
    config.setIndexatorSmeAddrChunkSizes(indexatorSmeAddrChunkSizes);
    changed = true;
  }
}
