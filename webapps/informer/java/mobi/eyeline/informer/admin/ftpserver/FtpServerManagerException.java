package mobi.eyeline.informer.admin.ftpserver;

import mobi.eyeline.informer.admin.AdminException;

/**
 * User: artem
 * Date: 25.01.11
 */
public class FtpServerManagerException extends AdminException {

  protected FtpServerManagerException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected FtpServerManagerException(String key, String... args) {
    super(key, args);
  }
}
