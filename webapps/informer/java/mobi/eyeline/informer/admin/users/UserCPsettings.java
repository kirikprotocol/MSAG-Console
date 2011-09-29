package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;

import java.io.Serializable;
import java.security.MessageDigest;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 29.11.2010
 * Time: 14:38:29
 */
public class UserCPsettings implements Serializable {
  private Protocol protocol;
  private WorkType workType;
  private String host;
  private Integer port;
  private String directory;
  private int directoryMaxSize;
  private String encoding;
  private Address sourceAddress;
  private String login;
  private String password;
  private String hashId;

  private Time activePeriodStart;
  private Time activePeriodEnd;

  private long periodInMin;

  private boolean createReports;

  private String name;

  private String workGroup;
  private boolean passiveMode;

  private Integer reportTimeoutMin;

  public UserCPsettings() {
  }

  public UserCPsettings(UserCPsettings other) {
    this.name = other.name;
    this.protocol = other.protocol;
    this.host = other.host;
    this.port = other.port;
    this.directory = other.directory;
    this.sourceAddress = other.sourceAddress==null ? null : new Address(other.sourceAddress);
    this.encoding = other.encoding;
    this.login = other.login;
    this.password = other.password;
    this.hashId=other.hashId;
    this.workType = other.workType;
    this.activePeriodEnd = other.activePeriodEnd;
    this.activePeriodStart = other.activePeriodStart;
    this.periodInMin = other.periodInMin;
    this.createReports = other.createReports;
    this.directoryMaxSize = other.directoryMaxSize;
    this.workGroup = other.workGroup;
    this.passiveMode = other.passiveMode;
    this.reportTimeoutMin = other.reportTimeoutMin;
  }

  public String getWorkGroup() {
    return workGroup;
  }

  public void setWorkGroup(String workGroup) {
    this.workGroup = workGroup;
  }

  public Integer getReportTimeoutMin() {
    return reportTimeoutMin;
  }

  public void setReportTimeoutMin(Integer reportTimeoutMin) {
    this.reportTimeoutMin = reportTimeoutMin;
  }

  public boolean isPassiveMode() {
    return passiveMode;
  }

  public void setPassiveMode(boolean passiveMode) {
    this.passiveMode = passiveMode;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public boolean isCreateReports() {
    return createReports;
  }

  public void setCreateReports(boolean createReports) {
    this.createReports = createReports;
  }

  public long getPeriodInMin() {
    return periodInMin;
  }

  public void setPeriodInMin(long periodInMin) {
    this.periodInMin = periodInMin;
  }

  public Time getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Time activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
  }

  public Time getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(Time activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
  }

  public WorkType getWorkType() {
    return workType;
  }

  public void setWorkType(WorkType workType) {
    this.workType = workType;
  }

  public String getHost() {
    return host;
  }

  public void setHost(String host) {
    this.host = host;
    hashId=null;
  }

  public Integer getPort() {
    return port;
  }

  public void setPort(Integer port) {
    this.port = port;
    hashId=null;
  }

  public String getDirectory() {
    return directory;
  }

  public void setDirectory(String directory) {
    this.directory = directory;
    hashId=null;
  }

  public Address getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(Address sourceAddress) {
    this.sourceAddress = sourceAddress;
    hashId=null;
  }

  public String getEncoding() {
    return encoding;
  }

  public void setEncoding(String encoding){
    this.encoding = encoding;
    hashId=null;
  }

  public String getLogin() {
    return login;
  }

  public void setLogin(String login) {
    this.login = login;
    hashId=null;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
    hashId=null;
  }

  public Protocol getProtocol() {
    return protocol;
  }

  public void setProtocol(Protocol protocol) {
    this.protocol = protocol;
  }

  public int getDirectoryMaxSize() {
    return directoryMaxSize;
  }

  public void setDirectoryMaxSize(int directoryMaxSize) {
    this.directoryMaxSize = directoryMaxSize;
  }
         
  
  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    UserCPsettings that = (UserCPsettings) o;

    if (createReports != that.createReports) return false;
    if (directoryMaxSize != that.directoryMaxSize) return false;
    if (periodInMin != that.periodInMin) return false;
    if (activePeriodEnd != null ? !activePeriodEnd.equals(that.activePeriodEnd) : that.activePeriodEnd != null)
      return false;
    if (activePeriodStart != null ? !activePeriodStart.equals(that.activePeriodStart) : that.activePeriodStart != null)
      return false;
    if (directory != null ? !directory.equals(that.directory) : that.directory != null) return false;
    if (workGroup != null ? !workGroup.equals(that.workGroup) : that.workGroup != null) return false;
    if(passiveMode != that.passiveMode) return false;
//    if (reportTimeoutMin != null ? !reportTimeoutMin.equals(that.reportTimeoutMin) : that.reportTimeoutMin != null) return false;
    if (encoding != null ? !encoding.equals(that.encoding) : that.encoding != null) return false;
    if (hashId != null ? !hashId.equals(that.hashId) : that.hashId != null) return false;
    if (host != null ? !host.equals(that.host) : that.host != null) return false;
    if (login != null ? !login.equals(that.login) : that.login != null) return false;
    if (name != null ? !name.equals(that.name) : that.name != null) return false;
    if (password != null ? !password.equals(that.password) : that.password != null) return false;
    if (port != null ? !port.equals(that.port) : that.port != null) return false;
    if (protocol != that.protocol) return false;
    if (sourceAddress != null ? !sourceAddress.equals(that.sourceAddress) : that.sourceAddress != null) return false;
    if (workType != that.workType) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = protocol != null ? protocol.hashCode() : 0;
    result = 31 * result + (workType != null ? workType.hashCode() : 0);
    result = 31 * result + (host != null ? host.hashCode() : 0);
    result = 31 * result + (port != null ? port.hashCode() : 0);
    result = 31 * result + (directory != null ? directory.hashCode() : 0);
    result = 31 * result + (workGroup != null ? workGroup.hashCode() : 0);
    result = 31 * result + (passiveMode ? 1 : 0);
    result = 31 * result + directoryMaxSize;
    result = 31 * result + (encoding != null ? encoding.hashCode() : 0);
//    result = 31 * result + (reportTimeoutMin != null ? reportTimeoutMin.hashCode() : 0);
    result = 31 * result + (sourceAddress != null ? sourceAddress.hashCode() : 0);
    result = 31 * result + (login != null ? login.hashCode() : 0);
    result = 31 * result + (password != null ? password.hashCode() : 0);
    result = 31 * result + (hashId != null ? hashId.hashCode() : 0);
    result = 31 * result + (activePeriodStart != null ? activePeriodStart.hashCode() : 0);
    result = 31 * result + (activePeriodEnd != null ? activePeriodEnd.hashCode() : 0);
    result = 31 * result + (int) (periodInMin ^ (periodInMin >>> 32));
    result = 31 * result + (createReports ? 1 : 0);
    result = 31 * result + (name != null ? name.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    if(login!=null) sb.append(login).append('@');
    sb.append(protocol).append("://");
    if(host!=null) sb.append(host);
    if(port!=null) sb.append(':').append(port.toString());
    if(directory!=null) {
      if(!directory.startsWith("/")) {
        sb.append("/");
      }
      sb.append(directory);
    }
    sb.append('(').
        append("workType=").append(workType).
        append(", activePeriodStart=").append(activePeriodStart).
        append(", activePeriodEnd=").append(activePeriodEnd).
        append(", periodInMin=").append(periodInMin).
        append(", createReports=").append(createReports).
        append(", name=").append(name).
        append(')');
    return sb.toString();
  }

  public String getHashId() throws AdminException {

    StringBuilder sb = new StringBuilder();
    if(login!=null) sb.append(login).append('@');
    sb.append(protocol).append("://");
    if(host!=null) sb.append(host);
    if(port!=null) sb.append(':').append(port.toString());
    if(directory!=null) {
      if(!directory.startsWith("/")) {
        sb.append("/");
      }
      sb.append(directory);
    }

    if(hashId==null) {
      try {
        byte[] bytes = MessageDigest.getInstance("MD5").digest(sb.toString().getBytes("UTF-8"));
        hashId="";
        for(byte b : bytes) {
          String s = Integer.toHexString(b&0xFF);
          if(s.length()==1) s="0"+s;
          hashId+=s;
        }
      }
      catch (Exception e) {
        throw new UserException("generalError",e);
      }
    }
    return hashId;
  }

  public void checkValid() throws UserException {
    if(protocol==null) throw new UserException("ucps.protocol.invalid");
    if(protocol != Protocol.file) {
      if(protocol != Protocol.localFtp) {
        if(host==null || host.length()==0)     throw new UserException("ucps.host.invalid");
      }else {
        if(directoryMaxSize<=0)     throw new UserException("ucps.directoryMaxSize.invalid");
      }
      if(protocol != Protocol.smb) {
        if(login==null|| login.length()==0)    throw new UserException("ucps.login.invalid");
        if(password==null || password.length()==0) throw new UserException("ucps.password.invalid");
      }
    }
    if(protocol != Protocol.localFtp) {
      if(directory==null || directory.length()==0) throw new UserException("ucps.directory.invalid");
    }
//    if(reportTimeoutMin != null && reportTimeoutMin<=0) throw new UserException("ucps.reportTimeoutMin.invalid");
    if(sourceAddress==null) throw new UserException("ucps.sourceaddr.invalid");
    if(workType == null) throw new UserException("ucps.workType.invalid");
    if(periodInMin <= 0) throw new UserException("ucps.periodInMin.invalid");
    if(activePeriodStart == null) throw new UserException("ucps.activePeriodStart.invalid");
    if(activePeriodEnd == null) throw new UserException("ucps.activePeriodEnd.invalid");
    if(name == null || name.length() == 0) throw new UserException("ucps.name.invalid");
  }


  public enum Protocol {
    sftp,file,smb,ftp,localFtp
  }

  public enum WorkType {
    simple, detailed, detailed_period
  }
}