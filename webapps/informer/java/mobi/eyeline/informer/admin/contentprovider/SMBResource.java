package mobi.eyeline.informer.admin.contentprovider;

import jcifs.smb.NtlmPasswordAuthentication;
import jcifs.smb.SmbFile;
import jcifs.smb.SmbNamedPipe;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class SMBResource implements FileResource{

  private static final Logger logger = Logger.getLogger(FTPResource.class);

  private FileSystem fileSysLocal;
  private UserCPsettings ucps;

  private SmbNamedPipe smb;

  public SMBResource(FileSystem fileSysLocal, UserCPsettings ucps) {
    this.fileSysLocal = fileSysLocal;
    this.ucps = ucps;
  }

  @Override
  public void open() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Open connection: "+ucps);
    }

    StringBuilder url = new StringBuilder().append("smb://").
        append(ucps.getHost());
    if(ucps.getPort() != null && ucps.getPort() != 0) {
      url.append(':').append(ucps.getPort());
    }

    if(!ucps.getDirectory().startsWith("/")) {
      url.append('/');
    }
    url.append(ucps.getDirectory());
    if(!ucps.getDirectory().endsWith("/")) {
      url.append('/');
    }

    try{
      if((ucps.getLogin() == null || ucps.getLogin().length() == 0) &&
          (ucps.getPassword() == null || ucps.getPassword().length() == 0)) {
        smb = new SmbNamedPipe( url.toString(), SmbNamedPipe.PIPE_TYPE_RDWR | SmbNamedPipe.PIPE_TYPE_CALL);
      }else {
        smb = new SmbNamedPipe(url.toString(), SmbNamedPipe.PIPE_TYPE_RDWR | SmbNamedPipe.PIPE_TYPE_CALL,
            new NtlmPasswordAuthentication(null, ucps.getLogin(), ucps.getPassword()));
      }
    }catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public List<String> listCSVFiles() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("List csv files: "+ucps);
    }
    List<String> result = new LinkedList<String>();
    try{
      String[] fs = smb.list();
      for(String _f : fs) {
        if(_f.endsWith(".csv")) {
          result.add(_f);
        }
      }
    }catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
    return result;
  }

  @Override
  public void get(String path, File localFile) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Get remote file: "+path+" to "+localFile.getAbsolutePath());
    }
    try{
      SmbFile file = new SmbFile(smb, path);
      InputStream is = null;
      OutputStream os = null;
      try{
        is = new BufferedInputStream(file.getInputStream());
        os = new BufferedOutputStream(fileSysLocal.getOutputStream(localFile, false));
        byte[] buffer = new byte[1024];
        int readed;
        while((readed = is.read(buffer)) > 0) {
          os.write(buffer, 0, readed);
        }
      }finally {
        if(is != null) {
          try{
            is.close();
          }catch (IOException ignored){}
        }
        if(os != null) {
          try{
            os.close();
          }catch (IOException ignored){}
        }
      }
    }catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Rename remote file from: "+fromPath+" to "+toPath);
    }
    try{
      new SmbFile(smb, fromPath).renameTo(new SmbFile(smb, toPath));
    }catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public void remove(String path) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Remove remote file: "+path);
    }
    try{
      new SmbFile(smb, path).delete();
    }catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public void put(File localFile, String toPath) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Upload file: "+localFile.getAbsolutePath()+" to "+toPath);
    }
    try{
      SmbFile remote = new SmbFile(smb, toPath);
      remote.createNewFile();
      InputStream is = null;
      OutputStream os = null;
      try{
        is = new BufferedInputStream(fileSysLocal.getInputStream(localFile));
        os = new BufferedOutputStream(remote.getOutputStream());
        byte[] buffer = new byte[1024];
        int readed;
        while((readed = is.read(buffer)) > 0) {
          os.write(buffer, 0, readed);
        }
      }finally {
        if(is != null) {
          try{
            is.close();
          }catch (IOException ignored){}
        }
        if(os != null) {
          try{
            os.close();
          }catch (IOException ignored){}
        }
      }
    }catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }

  }

  @Override
  public void close() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Close connection: "+ucps);
    }

  }
}
