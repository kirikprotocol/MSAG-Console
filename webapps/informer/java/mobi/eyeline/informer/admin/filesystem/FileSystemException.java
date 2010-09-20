package mobi.eyeline.informer.admin.filesystem;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class FileSystemException extends AdminException {
  protected FileSystemException(String key, Throwable cause) {
    super(key, cause);
  }

  protected FileSystemException(String key) {
    super(key);
  }

  protected FileSystemException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
