package mobi.eyeline.informer.admin;

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
    this.javaMailProperties = javaMailProperties;
  }
}
