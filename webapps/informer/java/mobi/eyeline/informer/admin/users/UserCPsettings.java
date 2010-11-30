package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import sun.security.provider.MD5;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 29.11.2010
 * Time: 14:38:29
 */
public class UserCPsettings {
  private String host;
  private int port;
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
    this.host = other.host;
    this.port = other.port;
    this.directory = other.directory;
    this.sourceAddress = new Address(other.sourceAddress);
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

  public int getPort() {
    return port;
  }

  public void setPort(int port) {
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

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    UserCPsettings that = (UserCPsettings) o;

    if (port != that.port) return false;
    if (directory != null ? !directory.equals(that.directory) : that.directory != null) return false;
    if (encoding != null ? !encoding.equals(that.encoding) : that.encoding != null) return false;
    if (host != null ? !host.equals(that.host) : that.host != null) return false;
    if (login != null ? !login.equals(that.login) : that.login != null) return false;
    if (password != null ? !password.equals(that.password) : that.password != null) return false;
    if (sourceAddress != null ? !sourceAddress.equals(that.sourceAddress) : that.sourceAddress != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = host != null ? host.hashCode() : 0;
    result = 31 * result + port;
    result = 31 * result + (directory != null ? directory.hashCode() : 0);
    result = 31 * result + (encoding != null ? encoding.hashCode() : 0);
    result = 31 * result + (sourceAddress != null ? sourceAddress.hashCode() : 0);
    result = 31 * result + (login != null ? login.hashCode() : 0);
    result = 31 * result + (password != null ? password.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    return login+"@"+host + ':' +port+"/"+ directory;        
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
}
