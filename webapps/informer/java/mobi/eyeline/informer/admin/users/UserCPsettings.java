package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import javax.persistence.Transient;
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
  private String host;
  private Integer port;
  private String directory;
  private String encoding;
  private Address sourceAddress;
  private String login;
  private String password;
  private String hashId;


  private final ValidationHelper vh = new ValidationHelper(User.class);


  public UserCPsettings() {
  }

  public UserCPsettings(UserCPsettings other) {
    this.protocol = other.protocol;
    this.host = other.host;
    this.port = other.port;
    this.directory = other.directory;
    this.sourceAddress = other.sourceAddress==null ? null : new Address(other.sourceAddress);
    this.encoding = other.encoding;
    this.login = other.login;
    this.password = other.password;
    this.hashId=other.hashId;
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

  public void setSourceAddress(Address sourceAddress) throws AdminException {
    vh.checkNotNull("sourceAddr", sourceAddress);
    this.sourceAddress = sourceAddress;
    hashId=null;
  }

  public String getEncoding() {
    return encoding;
  }

  public void setEncoding(String encoding) throws ValidationException {
    if(encoding!=null) {
      vh.checkSupportedEncoding("fileEncoding",encoding);
    }
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

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    UserCPsettings that = (UserCPsettings) o;

    if (directory != null ? !directory.equals(that.directory) : that.directory != null) return false;
    if (encoding != null ? !encoding.equals(that.encoding) : that.encoding != null) return false;
    if (host != null ? !host.equals(that.host) : that.host != null) return false;
    if (login != null ? !login.equals(that.login) : that.login != null) return false;
    if (password != null ? !password.equals(that.password) : that.password != null) return false;
    if (port != null ? !port.equals(that.port) : that.port != null) return false;
    if (protocol != that.protocol) return false;
    if (sourceAddress != null ? !sourceAddress.equals(that.sourceAddress) : that.sourceAddress != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = protocol != null ? protocol.hashCode() : 0;
    result = 31 * result + (host != null ? host.hashCode() : 0);
    result = 31 * result + (port != null ? port.hashCode() : 0);
    result = 31 * result + (directory != null ? directory.hashCode() : 0);
    result = 31 * result + (encoding != null ? encoding.hashCode() : 0);
    result = 31 * result + (sourceAddress != null ? sourceAddress.hashCode() : 0);
    result = 31 * result + (login != null ? login.hashCode() : 0);
    result = 31 * result + (password != null ? password.hashCode() : 0);
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
    return sb.toString();
  }

  public String getHashId() throws AdminException {

    if(hashId==null) {
      try {
        byte[] bytes = MessageDigest.getInstance("MD5").digest(toString().getBytes("UTF-8"));
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
    if(protocol==Protocol.sftp) {
      if(host==null || host.length()==0)     throw new UserException("ucps.host.invalid");
      if(login==null|| login.length()==0)    throw new UserException("ucps.login.invalid");
      if(password==null || password.length()==0) throw new UserException("ucps.password.invalid");
    }
    if(directory==null || directory.length()==0) throw new UserException("ucps.directory.invalid");
    if(encoding==null || encoding.length()==0) throw new UserException("ucps.encoding.invalid");
    if(sourceAddress==null) throw new UserException("ucps.sourceaddr.invalid");
  }


  public enum Protocol {
    sftp,file
  }
}
