package mobi.eyeline.informer.admin.infosme;

/**
 * Информер offline
 * @author Artem Snopkov
 */
public class OfflineException extends InfosmeException {
  OfflineException(String key, Throwable cause) {
    super(key, cause);
  }

  OfflineException(String key, String... args) {
    super(key, args);
  }
}
