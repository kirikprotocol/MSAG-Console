package com.eyeline.sponsored.config;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;

/**
 * User: artem
 * Date: 05.02.2008
 */

public class Config {

  private String handlerConfigFile;
  private String timezonesFile;
  private String routesFile;
  private String smppConfigFile;

  private String advertisingHost;
  private int advertisingPort;
  private int advertisingConnTimeout;

  private String storageDriver;
  private String storageUrl;
  private String storageLogin;
  private String storagePwd;
  private int storageConnTimeout;
  private int storagePoolSize;

  public Config (XmlConfig config) throws ConfigException {
    handlerConfigFile = config.getString("handlerConfigFile");
    timezonesFile = config.getString("timezonesFile");
    routesFile = config.getString("routesFile");
    smppConfigFile = config.getString("smppConfigFile");

    final XmlConfigSection adv = config.getSection("advertising");
    if (adv == null)
      throw new ConfigException("advertising section not found");

    advertisingHost = adv.getString("host");
    advertisingPort = adv.getInt("port");
    advertisingConnTimeout = adv.getInt("connectionTimeout");


    final XmlConfigSection storage = config.getSection("storage");
    if (storage == null)
      throw new ConfigException("storage section not found");

    storageDriver = storage.getString("driver");
    storageUrl = storage.getString("url");
    storageLogin = storage.getString("login");
    storagePwd = storage.getString("password");
    storageConnTimeout = storage.getInt("connectionTimeout");
    storagePoolSize = storage.getInt("poolSize");
  }

  public String getHandlerConfigFile() {
    return handlerConfigFile;
  }

  public String getTimezonesFile() {
    return timezonesFile;
  }

  public String getRoutesFile() {
    return routesFile;
  }

  public String getSmppConfigFile() {
    return smppConfigFile;
  }

  public String getAdvertisingHost() {
    return advertisingHost;
  }

  public int getAdvertisingPort() {
    return advertisingPort;
  }

  public int getAdvertisingConnTimeout() {
    return advertisingConnTimeout;
  }

  public String getStorageDriver() {
    return storageDriver;
  }

  public String getStorageUrl() {
    return storageUrl;
  }

  public String getStorageLogin() {
    return storageLogin;
  }

  public String getStoragePwd() {
    return storagePwd;
  }

  public int getStorageConnTimeout() {
    return storageConnTimeout;
  }

  public int getStoragePoolSize() {
    return storagePoolSize;
  }
}
