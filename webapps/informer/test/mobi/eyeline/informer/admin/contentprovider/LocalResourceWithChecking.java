package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

/**
* author: Aleksandr Khalitov
*/
class LocalResourceWithChecking extends FileResource {

  private final FileResource local;

  LocalResourceWithChecking(File dir, FileSystem fs) {
    this.local = FileResource.createLocal(dir, fs);
  }

  boolean opened = false;

  @Override
  public void open() throws AdminException {
    if(opened) {
      throw new IllegalStateException("Already opened!");
    }
    local.open();
    opened = true;
  }

  @Override
  public List<String> listCSVFiles() throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    return local.listCSVFiles();
  }

  @Override
  public boolean contains(String path) throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    return local.contains(path);
  }

  @Override
  public void get(String path, OutputStream os) throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    local.get(path, os);
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    local.rename(fromPath, toPath);
  }

  @Override
  public void remove(String path) throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    local.remove(path);
  }

  @Override
  public void put(InputStream is, String toPath) throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    local.put(is, toPath);
  }

  @Override
  public void close() throws AdminException {
    if(!opened) {
      throw new IllegalStateException("Resource is closed!!!");
    }
    local.close();
    opened = false;
  }
}
