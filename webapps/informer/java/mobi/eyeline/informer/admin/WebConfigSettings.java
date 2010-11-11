package mobi.eyeline.informer.admin;

import java.io.File;
import java.util.Collection;

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


  public InstallationType getInstallationType() {
    return installationType;
  }

  void setInstallationType(InstallationType installationType) {
    this.installationType = installationType;
  }


  public String getSingleDaemonHost() {
    return daemonHost;
  }

  void setDaemonHost(String daemonHost) {
    this.daemonHost = daemonHost;
  }


  public Integer getSingleDaemonPort() {
    return daemonPort;
  }

  void setDaemonPort(Integer daemonPort) {
    this.daemonPort = daemonPort;
  }


  public File[] getAppMirrorDirs() {
    return appMirrorDirs;
  }

  void setAppMirrorDirs(File[] appMirrorDirs) {
    this.appMirrorDirs = appMirrorDirs;
  }

  public String getHSDaemonHost() {
    return daemonHost;
  }


  public Integer getHSDaemonPort() {
    return daemonPort;
  }


  public Collection<String> getHSDaemonHosts() {
    return HSDaemonHosts;
  }

  void setHSDaemonHosts(Collection<String> HSDaemonHosts) {
    this.HSDaemonHosts = HSDaemonHosts;
  }


  public String getJournalDir() {
    return journalDir;
  }

  void setJournalDir(String journalDir) {
    this.journalDir = journalDir;
  }


}
