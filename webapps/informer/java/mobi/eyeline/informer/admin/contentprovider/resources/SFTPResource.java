package mobi.eyeline.informer.admin.contentprovider.resources;

import com.jcraft.jsch.*;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderException;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 30.11.2010
 * Time: 17:00:06
 */
class SFTPResource extends FileResource {

  private JSch jsch;
  private Session session = null;
  private ChannelSftp channel = null;

  private final String host;
  private final Integer port;
  private final String login;
  private final String password;
  private final String remoteDir;


  SFTPResource(String host, Integer port, String login, String password, String remoteDir)  {
    this.host = host;
    this.port = port;
    this.login = login;
    this.password = password;
    this.remoteDir = remoteDir;

    jsch = new JSch();
    jsch.setHostKeyRepository(new HostKeyRepository() {
      @Override
      public int check(String s, byte[] bytes) {
        return 0;
      }

      @Override
      public void add(HostKey hostKey, UserInfo userInfo) {
      }

      @Override
      public void remove(String s, String s1) {
      }

      @Override
      public void remove(String s, String s1, byte[] bytes) {
      }

      @Override
      public String getKnownHostsRepositoryID() {
        return null;
      }

      @Override
      public HostKey[] getHostKey() {
        return new HostKey[0];
      }

      @Override
      public HostKey[] getHostKey(String s, String s1) {
        return new HostKey[0];
      }
    });
  }


  public void open() throws AdminException {
    try {
      session = jsch.getSession(login, host);
      if(port != null && port != 0) session.setPort(port);
      session.setPassword(password);
      session.setTimeout(30000);
      session.connect();
      channel = (ChannelSftp)session.openChannel("sftp");
      channel.connect();
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public List<String> listCSVFiles() throws AdminException {
    try {
      List<String> remoteFiles = new ArrayList<String>();
      for (Object o : channel.ls(remoteDir+ "/*.csv")) {
        ChannelSftp.LsEntry entry = (ChannelSftp.LsEntry)o;
        remoteFiles.add(entry.getFilename());
      }
      return remoteFiles;
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public void get(String fileName, OutputStream os) throws AdminException {
    try {
      channel.get(remoteDir + "/" + fileName,os);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public void rename(String fromFileName, String toFileName) throws AdminException {
    try {
      channel.rename(remoteDir + "/" +fromFileName, remoteDir + "/" +toFileName);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void remove(String path) throws AdminException {
    try {
      channel.rm(remoteDir + "/" + path);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public void put(InputStream is, String toFileName) throws AdminException {
    try {
      channel.put(is, remoteDir + "/" +toFileName);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }


  public void close() throws AdminException{
    if (session != null) try {session.disconnect(); } catch (Exception ignored){}
    if (channel != null) try {channel.exit(); } catch (Exception ignored){}
    session=null;
    channel=null;
  }

  public String toString() {
    return "sftp://" + login + '@' + host + ':' + port + '/' + remoteDir;
  }
}
