package ru.novosoft.smsc.admin.filesystem;

import ru.novosoft.smsc.admin.AdminException;

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
