package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;

import java.io.File;

/**
 * User: artem
 * Date: 28.01.11
 */
class ResourceOptions {

  File workDir;
  User user;
  Address sourceAddress;
  String encoding;
  boolean createReports;
  String host;
  int port;
  UserCPsettings.Protocol protocol;


  ResourceOptions(User u, File workDir, UserCPsettings s) {
    this.user = u;
    this.workDir = workDir;
    this.sourceAddress = s.getSourceAddress();
    this.encoding = s.getEncoding();
    this.createReports = s.isCreateReports();
    this.host = s.getHost();
    this.port = s.getPort();
    this.protocol = s.getProtocol();
  }

  public String getHost() {
    return host;
  }

  public int getPort() {
    return port;
  }

  public UserCPsettings.Protocol getProtocol() {
    return protocol;
  }

  public boolean isCreateReports() {
    return createReports;
  }

  public File getWorkDir() {
    return workDir;
  }

  public void setWorkDir(File workDir) {
    this.workDir = workDir;
  }

  public User getUser() {
    return user;
  }

  public void setUser(User user) {
    this.user = user;
  }

  public Address getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(Address sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public String getEncoding() {
    return encoding;
  }

  public void setEncoding(String encoding) {
    this.encoding = encoding;
  }
}
