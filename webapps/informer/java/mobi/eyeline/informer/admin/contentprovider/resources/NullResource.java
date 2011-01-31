package mobi.eyeline.informer.admin.contentprovider.resources;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collections;
import java.util.List;

/**
 * User: artem
 * Date: 25.01.11
 */
class NullResource extends FileResource {
  @Override
  public void open() throws AdminException {
  }

  @Override
  public List<String> listCSVFiles() throws AdminException {
    return Collections.emptyList();
  }

  @Override
  public void get(String path, OutputStream os) throws AdminException {
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
  }

  @Override
  public void remove(String path) throws AdminException {
  }

  @Override
  public void put(InputStream is, String toPath) throws AdminException {
  }

  @Override
  public void close() throws AdminException {
  }
}
