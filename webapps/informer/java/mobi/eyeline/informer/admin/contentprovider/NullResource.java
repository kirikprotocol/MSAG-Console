package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;
import java.util.Collections;
import java.util.List;

/**
 * User: artem
 * Date: 25.01.11
 */
class NullResource implements FileResource {
  @Override
  public void open() throws AdminException {
  }

  @Override
  public List<String> listCSVFiles() throws AdminException {
    return Collections.emptyList();
  }

  @Override
  public void get(String path, File localFile) throws AdminException {
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
  }

  @Override
  public void remove(String path) throws AdminException {
  }

  @Override
  public void put(File localFile, String toPath) throws AdminException {
  }

  @Override
  public void close() throws AdminException {
  }
}
