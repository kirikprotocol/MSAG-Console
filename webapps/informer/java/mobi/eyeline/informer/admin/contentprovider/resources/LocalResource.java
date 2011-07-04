package mobi.eyeline.informer.admin.contentprovider.resources;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

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
class LocalResource extends FileResource {

  private FileSystem fileSys;
  private File dir;

  public LocalResource(File dir, FileSystem fs) {
    this.fileSys = fs;
    this.dir = dir;
  }

  public LocalResource(File informerBase, FileSystem fileSys, String dir) {
    this.fileSys = fileSys;
    if(dir.indexOf(File.separatorChar)==0) {
      this.dir = new File(dir);
    } else {
      this.dir = new File(informerBase,dir);
    }
  }

  public void open() throws AdminException {
    if (!fileSys.exists(dir))
      throw new ContentProviderException("userDirNotFound","", dir.getAbsolutePath());
  }

  public List<String> listCSVFiles() throws AdminException {
    String[] files = fileSys.list(dir);
    List<String> ret = new ArrayList<String>();
    for(String f: files) {
      if(f.endsWith(".csv")) {
        ret.add(f);
      }
    }
    return ret;
  }

  public void get(String fileName, OutputStream os) throws AdminException {
    InputStream is = null;
    try {
      is = fileSys.getInputStream(new File(dir, fileName));
      pump(is,os);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }
  }

  @Override
  public boolean contains(String path) throws AdminException {
    return fileSys.exists(new File(dir, path));
  }


  public void rename(String fromFileName, String toFileName) throws AdminException {
    fileSys.rename(new File(dir,fromFileName),new File(dir,toFileName));
  }

  @Override
  public void remove(String path) throws AdminException {
    fileSys.delete(new File(dir, path));
  }

  public void put(InputStream is, String fileName) throws AdminException {
    OutputStream os = null;
    try {
      os = fileSys.getOutputStream(new File(dir,fileName),false);
      pump(is,os);
    }
    catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
    finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException ignored) {
        }
    }
  }

  public void close() throws AdminException {
  }

  private void pump(InputStream is, OutputStream os) throws IOException {
    byte[] buf = new byte[4096];
    int nread=0;
    while((nread=is.read(buf,0,buf.length))>=0) {
      os.write(buf,0,nread);
    }
  }

  public String toString() {
    return "file://" + dir;
  }

}
