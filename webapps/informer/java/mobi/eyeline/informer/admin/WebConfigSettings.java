package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.util.Address;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.11.2010
 * Time: 15:09:49
 */
class WebConfigSettings {
  private InstallationType installationType;
  private String daemonHost;
  private Integer daemonPort;
  private File[] appMirrorDirs;
  private Collection<String> HSDaemonHosts;
  private String journalDir;
  private Properties javaMailProperties;
  private Address smsSenderAddress;
  private Properties notificationTemplates;
  private Properties siebelProperties;
  private boolean allowUssdPushDeliveries;

  InstallationType getInstallationType() {
    return installationType;
  }

  void setInstallationType(InstallationType installationType) {
    this.installationType = installationType;
  }


  String getSingleDaemonHost() {
    return daemonHost;
  }

  void setDaemonHost(String daemonHost) {
    this.daemonHost = daemonHost;
  }


  Integer getSingleDaemonPort() {
    return daemonPort;
  }

  void setDaemonPort(Integer daemonPort) {
    this.daemonPort = daemonPort;
  }


  File[] getAppMirrorDirs() {
    File[] ret = new File[appMirrorDirs.length];
    System.arraycopy(appMirrorDirs, 0, ret, 0, ret.length);
    return ret;
  }

  void setAppMirrorDirs(File[] appMirrorDirs) {
    this.appMirrorDirs = appMirrorDirs;
  }

  String getHSDaemonHost() {
    return daemonHost;
  }


  Integer getHSDaemonPort() {
    return daemonPort;
  }


  Collection<String> getHSDaemonHosts() {
    return new ArrayList<String>(HSDaemonHosts);
  }

  void setHSDaemonHosts(Collection<String> HSDaemonHosts) {
    this.HSDaemonHosts = HSDaemonHosts;
  }


  String getJournalDir() {
    return journalDir;
  }

  void setJournalDir(String journalDir) {
    this.journalDir = journalDir;
  }


  Properties getJavaMailProperties() {
    return cloneProps(javaMailProperties);
  }

  void setJavaMailProperties(Properties javaMailProperties) {
    validateJavaMailProperties(javaMailProperties);
    this.javaMailProperties = cloneProps(javaMailProperties);
  }

  Address getSmsSenderAddress() {
    return new Address(smsSenderAddress);
  }

  void setSmsSenderAddress(Address smsSenderAddress) {
    this.smsSenderAddress = new Address(smsSenderAddress);
  }

  void setNotificationTemplates(Properties notificationTemplates) {
    validateNotificationTemplates(notificationTemplates);
    this.notificationTemplates =cloneProps(notificationTemplates);
  }

  Properties getNotificationTemplates() {
    return cloneProps(notificationTemplates);
  }

  void setSiebelProperties(Properties siebelProperties) {
    this.siebelProperties =cloneProps(siebelProperties);
  }

  Properties getSiebelProperties() {
    return cloneProps(siebelProperties);
  }

  boolean isAllowUssdPushDeliveries() {
    return allowUssdPushDeliveries;
  }

  void setAllowUssdPushDeliveries(boolean allowUssdPushDeliveries) {
    this.allowUssdPushDeliveries = allowUssdPushDeliveries;
  }

  private void validateNotificationTemplates(Properties notificationTemplates) {
    //todo check required properties
  }
  private void validateJavaMailProperties(Properties javaMailProperties) {
    //todo check required properties
  }

  private Properties cloneProps(Properties other) {
    Properties props = new Properties();
    for(Object s : other.keySet()) {
      props.put(s,other.get(s));
    }
    return props;
  }
}
