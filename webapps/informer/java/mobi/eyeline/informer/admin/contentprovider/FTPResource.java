package mobi.eyeline.informer.admin.contentprovider;

import it.sauronsoftware.ftp4j.FTPClient;
import it.sauronsoftware.ftp4j.FTPFile;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class FTPResource implements FileResource{

  private static final Logger logger = Logger.getLogger(FTPResource.class);

  private FileSystem fileSysLocal;
  private UserCPsettings ucps;

  private FTPClient ftp;

  public FTPResource(FileSystem fileSysLocal, UserCPsettings ucps) {
    this.fileSysLocal = fileSysLocal;
    this.ucps = ucps;
  }

  @Override
  public void open() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Open connection: "+ucps);
    }
    try{
      ftp = new FTPClient();
      ftp.setType(FTPClient.TYPE_TEXTUAL);
      if(ucps.getPort() == null || ucps.getPort() == 0) {
        ftp.connect(ucps.getHost());
      }else {
        ftp.connect(ucps.getHost(), ucps.getPort());
      }
      ftp.setCharset("windows-1251");
      ftp.login(ucps.getLogin(), ucps.getPassword());
      ftp.changeDirectory(ucps.getDirectory());
    } catch (Exception e) {
      logger.error(e,e);
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public List<String> listCSVFiles() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("List csv files: "+ucps);
    }
    List<String> result = new LinkedList<String>();
    try{
      FTPFile[] fs = ftp.list();
      for(FTPFile f : fs) {
        String name = f.getName();
        if(name.endsWith(".csv")) {
          result.add(name);
        }
      }
    } catch (Exception e) {
      logger.error(e,e);
      throw new ContentProviderException("connectionError",e);
    }
    return result;
  }

  @Override
  public void get(String path, File localFile) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Get remote file: "+path+" to "+localFile.getAbsolutePath());
    }
    OutputStream os = null;
    try{
      os = fileSysLocal.getOutputStream(localFile, false);
      ftp.download(path, os, 0, null);
    } catch (Exception e) {
      logger.error(e,e);
      throw new ContentProviderException("connectionError",e);
    } finally {
      if(os != null) {
        try{
          os.close();
        }catch (IOException ignored){}
      }
    }
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Rename remote file: "+fromPath+" to "+toPath);
    }
    try{
      ftp.rename(fromPath, toPath);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void remove(String path) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Remove remote file: "+path);
    }
    try{
      ftp.deleteFile(path);
    } catch (Exception e) {
      logger.error(e,e);
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void put(File localFile, String toPath) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Upload file: "+localFile.getAbsolutePath()+" to remote: "+toPath);
    }
    InputStream is = null;
    try{
      is = fileSysLocal.getInputStream(localFile);
      ftp.upload(toPath, is, 0, 0, null);
    }catch (Exception e){
      logger.error(e,e);
      throw new ContentProviderException("connectionError",e);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException ignored){}
      }
    }
  }



  @Override
  public void close() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Close connection: "+ucps);
    }
    if(ftp != null) {
      try{
        ftp.disconnect(true);
      } catch (Exception e) {
        throw new ContentProviderException("connectionError",e);
      }
    }
  }
}
