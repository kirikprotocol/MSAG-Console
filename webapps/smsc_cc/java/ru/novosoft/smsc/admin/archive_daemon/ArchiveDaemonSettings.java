package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.util.HashMap;
import java.util.Map;

/**
 * Структура для хранения настроек ArchiveDaemon-а. Так же выполняет функцию по проверке корректности настроек.
 * @author Artem Snopkov
 */
public class ArchiveDaemonSettings {

  private final static ValidationHelper vh = new ValidationHelper(ArchiveDaemonSettings.class);

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
  private Map<String, String> locationsSources = new HashMap<String, String>();

  private int indexatorMaxFlushSpeed;
  private Map<String, Integer> indexatorSmeAddrChunkSizes = new HashMap<String, Integer>();

  ArchiveDaemonSettings() {
  }

  ArchiveDaemonSettings(ArchiveDaemonSettings c) {
    interval = c.interval;
    viewHost = c.viewHost;
    viewPort = c.viewPort;
    viewTimeout = c.viewTimeout;
    queriesMax = c.queriesMax;
    queriesInit = c.queriesInit;
    transactionsMaxSmsCount = c.transactionsMaxSmsCount;
    transactionsMaxTimeInterval = c.transactionsMaxTimeInterval;
    locationsBaseDestination = c.locationsBaseDestination;
    locationsTextDestinations = c.locationsTextDestinations;
    locationsSources = new HashMap<String, String>(c.locationsSources);
    indexatorMaxFlushSpeed = c.indexatorMaxFlushSpeed;
    indexatorSmeAddrChunkSizes = new HashMap<String, Integer>(c.indexatorSmeAddrChunkSizes);
  }

  public int getInterval() {
    return interval;
  }

  public void setInterval(int interval) throws AdminException {
    vh.checkPositive("interval", interval);
    this.interval = interval;
  }

  public String getViewHost() {
    return viewHost;
  }

  public void setViewHost(String viewHost) {
    this.viewHost = viewHost;
  }

  public int getViewPort() {
    return viewPort;
  }

  public void setViewPort(int viewPort) {
    this.viewPort = viewPort;
  }

  public int getViewTimeout() {
    return viewTimeout;
  }

  public void setViewTimeout(int viewTimeout) throws AdminException {
    vh.checkPositive("viewTimeout", viewTimeout);
    this.viewTimeout = viewTimeout;
  }

  public int getQueriesMax() {
    return queriesMax;
  }

  public void setQueriesMax(int queriesMax) throws AdminException {
    vh.checkPositive("queriesMax", queriesMax);
    this.queriesMax = queriesMax;
  }

  public int getQueriesInit() {
    return queriesInit;
  }

  public void setQueriesInit(int queriesInit) throws AdminException {
    vh.checkGreaterOrEqualsTo("queriesInit", queriesInit, 0);
    this.queriesInit = queriesInit;
  }

  public int getTransactionsMaxSmsCount() {
    return transactionsMaxSmsCount;
  }

  public void setTransactionsMaxSmsCount(int transactionsMaxSmsCount) throws AdminException {
    vh.checkPositive("transactionsMaxSmsCount", transactionsMaxSmsCount);
    this.transactionsMaxSmsCount = transactionsMaxSmsCount;
  }

  public int getTransactionsMaxTimeInterval() {
    return transactionsMaxTimeInterval;
  }

  public void setTransactionsMaxTimeInterval(int transactionsMaxTimeInterval) throws AdminException {
    vh.checkPositive("transactionsMaxTimeInterval", transactionsMaxTimeInterval);
    this.transactionsMaxTimeInterval = transactionsMaxTimeInterval;
  }

  public String getLocationsBaseDestination() {
    return locationsBaseDestination;
  }

  public void setLocationsBaseDestination(String locationsBaseDestination) throws AdminException {
    vh.checkNotEmpty("locationsBaseDestination", locationsBaseDestination);
    this.locationsBaseDestination = locationsBaseDestination;
  }

  public String getLocationsTextDestinations() {
    return locationsTextDestinations;
  }

  public void setLocationsTextDestinations(String locationsTextDestinations) throws AdminException {
    vh.checkNotEmpty("locationsTextDestinations", locationsTextDestinations);
    this.locationsTextDestinations = locationsTextDestinations;
  }

  public Map<String, String> getLocationsSources() {
    return locationsSources;
  }

  public void setLocationsSources(Map<String, String> locationsSources) throws AdminException {
    vh.checkNoNulls("locationsSources", locationsSources);
    this.locationsSources = locationsSources;
  }

  public int getIndexatorMaxFlushSpeed() {
    return indexatorMaxFlushSpeed;
  }

  public void setIndexatorMaxFlushSpeed(int indexatorMaxFlushSpeed) throws AdminException {
    vh.checkPositive("indexatorMaxFlushSpeed", indexatorMaxFlushSpeed);
    this.indexatorMaxFlushSpeed = indexatorMaxFlushSpeed;
  }

  public Map<String, Integer> getIndexatorSmeAddrChunkSizes() {
    return indexatorSmeAddrChunkSizes;
  }

  public void setIndexatorSmeAddrChunkSizes(Map<String, Integer> indexatorSmeAddrChunkSizes) throws AdminException {
    vh.checkNoNulls("indexatorSmeAddrChunkSizes", indexatorSmeAddrChunkSizes);
    this.indexatorSmeAddrChunkSizes = indexatorSmeAddrChunkSizes;
  }
}
