package ru.novosoft.smsc.web.beans;

import java.io.Serializable;

/**
 * author: alkhal
 */
public class InstanceConfig implements Serializable {

  private int instanceNumber;

  private String adminHost;

  private int adminPort;

  private String corePerfHost;

  private int corePerfPort;

  private String smePerfHost;

  private int smePerfPort;

  private String messageStoreStatsDir;

  private String messageStoreArchDir;

  private String localStoreFilename;

  private String localStoreMessIdFile;

  private String smppHost;

  private int smppPort;

  private String agentHost;

  private int agentPort;

  private String cacheDir;

  public int getInstanceNumber() {
    return instanceNumber;
  }

  public void setInstanceNumber(int instanceNumber) {
    this.instanceNumber = instanceNumber;
  }

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String adminHost) {
    this.adminHost = adminHost;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) {
    this.adminPort = adminPort;
  }

  public String getCorePerfHost() {
    return corePerfHost;
  }

  public void setCorePerfHost(String corePerfHost) {
    this.corePerfHost = corePerfHost;
  }

  public int getCorePerfPort() {
    return corePerfPort;
  }

  public void setCorePerfPort(int corePerfPort) {
    this.corePerfPort = corePerfPort;
  }

  public String getSmePerfHost() {
    return smePerfHost;
  }

  public void setSmePerfHost(String smePerfHost) {
    this.smePerfHost = smePerfHost;
  }

  public int getSmePerfPort() {
    return smePerfPort;
  }

  public void setSmePerfPort(int smePerfPort) {
    this.smePerfPort = smePerfPort;
  }

  public String getMessageStoreStatsDir() {
    return messageStoreStatsDir;
  }

  public void setMessageStoreStatsDir(String messageStoreStatsDir) {
    this.messageStoreStatsDir = messageStoreStatsDir;
  }

  public String getMessageStoreArchDir() {
    return messageStoreArchDir;
  }

  public void setMessageStoreArchDir(String messageStoreArchDir) {
    this.messageStoreArchDir = messageStoreArchDir;
  }

  public String getLocalStoreFilename() {
    return localStoreFilename;
  }

  public void setLocalStoreFilename(String localStoreFilename) {
    this.localStoreFilename = localStoreFilename;
  }

  public String getLocalStoreMessIdFile() {
    return localStoreMessIdFile;
  }

  public void setLocalStoreMessIdFile(String localStoreMessIdFile) {
    this.localStoreMessIdFile = localStoreMessIdFile;
  }

  public String getSmppHost() {
    return smppHost;
  }

  public void setSmppHost(String smppHost) {
    this.smppHost = smppHost;
  }

  public int getSmppPort() {
    return smppPort;
  }

  public void setSmppPort(int smppPort) {
    this.smppPort = smppPort;
  }

  public String getAgentHost() {
    return agentHost;
  }

  public void setAgentHost(String agentHost) {
    this.agentHost = agentHost;
  }

  public int getAgentPort() {
    return agentPort;
  }

  public void setAgentPort(int agentPort) {
    this.agentPort = agentPort;
  }

  public String getCacheDir() {
    return cacheDir;
  }

  public void setCacheDir(String cacheDir) {
    this.cacheDir = cacheDir;
  }
}
