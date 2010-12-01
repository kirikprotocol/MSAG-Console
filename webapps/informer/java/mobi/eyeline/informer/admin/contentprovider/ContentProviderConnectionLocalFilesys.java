package mobi.eyeline.informer.admin.contentprovider;

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
 * Time: 18:34:53
 */
public class ContentProviderConnectionLocalFilesys implements ContentProviderConnection {
  private FileSystem fileSys;
  private File userSrcDir;

  public ContentProviderConnectionLocalFilesys(File informerBase, FileSystem fileSys, UserCPsettings ucps) {    
    this.fileSys = fileSys;
    if(ucps.getDirectory().indexOf(File.separatorChar)==0) {
      this.userSrcDir = new File(ucps.getDirectory());
    }
    else {
      this.userSrcDir = new File(informerBase,ucps.getDirectory());
    }
  }

  public void connect() throws AdminException {

  }

  public List<String> listCSVFiles() throws AdminException {
    String[] files = fileSys.list(userSrcDir);
    if(files==null) {
      throw new ContentProviderException("userDirNotFound","",userSrcDir.getAbsolutePath());
    }
    List<String> ret = new ArrayList();
    for(String f: files) {
      if(f.endsWith(".csv")) {
        ret.add(f);
      }
    }
    return ret;
  }

  public void get(String fileName, File localFile) throws AdminException {
    InputStream is = null;
    OutputStream os = null;
    try {
      is = fileSys.getInputStream(new File(userSrcDir,fileName));
      os = fileSys.getOutputStream(localFile,false);
      pump(is,os);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
    finally {
      closeStreams(is, os);
    }

  }


  public void rename(String fromFileName, String toFileName) throws AdminException {
    fileSys.rename(new File(userSrcDir,fromFileName),new File(userSrcDir,toFileName));
  }

  public void put(File localFile, String fileName) throws AdminException {
    InputStream is = null;
    OutputStream os = null;
    try {
      is = fileSys.getInputStream(localFile);
      os = fileSys.getOutputStream(new File(userSrcDir,fileName),false);
      pump(is,os);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
    finally {
      closeStreams(is, os);
    }
  }

  private void closeStreams(InputStream is, OutputStream os) {
    try {if(is!=null) is.close();} catch (Exception e){}
    try {if(os!=null) os.close();} catch (Exception e){}
  }

  public void close() throws AdminException {
    //To change body of implemented methods use File | Settings | File Templates.
  }

  private void pump(InputStream is, OutputStream os) throws IOException {
    byte[] buf = new byte[4096];
    int nread=0;
    while((nread=is.read(buf,0,buf.length))>=0) {
      os.write(buf,0,nread);
    }
  }

}
