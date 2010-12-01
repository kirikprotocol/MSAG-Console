package mobi.eyeline.informer.admin.contentprovider;

import com.jcraft.jsch.ChannelSftp;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.UserCPsettings;

import java.io.File;
import java.io.IOException;
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
class ContentProviderConnectionSFTP implements ContentProviderConnection {
  private JSch jsch;
  private Session session = null;
  private ChannelSftp channel = null;
  private FileSystem fileSysLocal;
  private UserCPsettings ucps;


  ContentProviderConnectionSFTP(FileSystem fileSysLocal,UserCPsettings ucps) throws AdminException {
    this.fileSysLocal = fileSysLocal;
    this.ucps = ucps;
    jsch = new JSch();
    try {
      String knownHostsFilename = System.getProperty("user.home") + "/.ssh/known_hosts";
      jsch.setKnownHosts(fileSysLocal.getInputStream(new File(knownHostsFilename)));
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }


  public void connect() throws AdminException {
    try {
      session = jsch.getSession(ucps.getLogin(), ucps.getHost());
      if(ucps.getPort()!=null) session.setPort(ucps.getPort());
      session.setPassword(ucps.getPassword());
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
      for (Object o : channel.ls(ucps.getDirectory()+ "/*.csv")) {
        ChannelSftp.LsEntry entry = (ChannelSftp.LsEntry)o;
        remoteFiles.add(entry.getFilename());
      }
      return remoteFiles;
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public void get(String fileName, File localFile) throws AdminException {
    OutputStream os = null;
    try {
      os = fileSysLocal.getOutputStream(localFile,false);
      channel.get(ucps.getDirectory() + "/" + fileName,os);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
    finally {
      if(os!=null) try {os.close();} catch (Exception ee) {}
    }
  }

  public void rename(String fromFileName, String toFileName) throws AdminException {
    try {
      channel.rename(ucps.getDirectory() + "/" +fromFileName,ucps.getDirectory() + "/" +toFileName);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public void put(File localFile, String toFileName) throws AdminException {
    InputStream is = null;
    try {
      is = fileSysLocal.getInputStream(localFile);
      channel.put(is,ucps.getDirectory() + "/" +toFileName);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
    finally {
      if(is!=null) try { is.close(); } catch (IOException e1) {}
    }
  }


  public void close() throws AdminException{
    if (session != null) try {session.disconnect(); } catch (Exception e){};
    if (channel != null) try {channel.exit(); } catch (Exception e){}
    session=null;
    channel=null;
  }


}
