package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.util.LocalizedException;

/**
 * @author Artem Snopkov
 */
public class UploadFileException extends LocalizedException {

  protected UploadFileException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected UploadFileException(String key, String... args) {
    super(key, args);
  }
}
