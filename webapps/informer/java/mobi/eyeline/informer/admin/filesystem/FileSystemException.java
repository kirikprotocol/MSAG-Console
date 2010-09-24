package mobi.eyeline.informer.admin.filesystem;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка работы с файловой системой
 * @author Artem Snopkov
 */
class FileSystemException extends AdminException {
  FileSystemException(String key, Throwable cause) {
    super(key, cause);
  }

  protected FileSystemException(String key) {
    super(key);
  }

  FileSystemException(String key, String ... args) {
    super(key, args);
  }
}
