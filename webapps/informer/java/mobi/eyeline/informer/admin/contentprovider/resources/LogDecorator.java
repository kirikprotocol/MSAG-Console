package mobi.eyeline.informer.admin.contentprovider.resources;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

/**
 * Добавляет логирование вызовов методов FileResource
 *
 * User: artem
 * Date: 28.01.11
 */
class LogDecorator extends FileResource {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private final FileResource wrapped;

  LogDecorator(FileResource wrapped) {
    this.wrapped = wrapped;
  }

  private void log(String text) {
    if (log.isDebugEnabled())
      log.debug(toString() + ": " + text);
  }

  @Override
  public void open() throws AdminException {
    log("openning ...");
    wrapped.open();
    log("OK");
  }

  @Override
  public List<String> listFiles() throws AdminException {
    log("listing ...");
    List<String> res = wrapped.listFiles();
    if (res.isEmpty()) {
      log("resource is empty.");
    } else {
      StringBuilder sb = new StringBuilder("\n" + res.size() + " files:");
      for (String f : res)
        sb.append("\n  ").append(f);
      log(sb.toString());
    }
    return res;
  }

  @Override
  public void get(String path, OutputStream os) throws AdminException {
    log("downloading " + path + " ...");
    wrapped.get(path, os);
    log("OK");
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    log("renaming " + fromPath + " to " + toPath + " ...");
    wrapped.rename(fromPath, toPath);
    log("OK");
  }

  @Override
  public void remove(String path) throws AdminException {
    log("removing " + path + " ...");
    wrapped.remove(path);
    log("OK");
  }

  @Override
  public void put(InputStream is, String toPath) throws AdminException {
    log("uploading " + toPath + " ...");
    wrapped.put(is, toPath);
    log("OK");
  }

  @Override
  public void close() throws AdminException {
    log("closing ...");
    wrapped.close();
    log("OK");
  }

  @Override
  public String toString() {
    return wrapped.toString();
  }
}
