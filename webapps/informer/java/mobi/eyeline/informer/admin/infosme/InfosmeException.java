package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class InfosmeException extends AdminException {

  InfosmeException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  InfosmeException(String key, Throwable cause) {
    super(key, cause);
  }

  InfosmeException(String key, String... args) {
    super(key, args);
  }
}
