package mobi.eyeline.informer.admin.infosme;

/**
 * @author Artem Snopkov
 */
public class OfflineException extends InfosmeException {
  public OfflineException(String key, Throwable cause) {
    super(key, cause);
  }

  public OfflineException(String key, String... args) {
    super(key, args);
  }
}
