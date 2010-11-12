package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.util.Address;

import java.io.File;
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
    return appMirrorDirs;
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
    return HSDaemonHosts;
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
    Properties props = new Properties();
    for(Object s : javaMailProperties.keySet()) {
      props.put(s,javaMailProperties.get(s));
    }
    return props;
  }

  void setJavaMailProperties(Properties javaMailProperties) {
    validateJavaMailProperties(javaMailProperties);
    this.javaMailProperties = javaMailProperties;
  }

  Address getSmsSenderAddress() {
    return smsSenderAddress;
  }

  void setSmsSenderAddress(Address smsSenderAddress) {
    this.smsSenderAddress = smsSenderAddress;
  }

  void setNotificationTemplates(Properties notificationTemplates) {
    validateNotificationTemplates(notificationTemplates);
    this.notificationTemplates =notificationTemplates;
  }

  Properties getNotificationTemplates() {
    Properties props = new Properties();
    for(Object s : notificationTemplates.keySet()) {
      props.put(s,notificationTemplates.get(s));
    }
    return props;
  }


  private void validateNotificationTemplates(Properties notificationTemplates) {
    //todo check required properties
  }
  private void validateJavaMailProperties(Properties javaMailProperties) {
    //todo check required properties
  }
}
