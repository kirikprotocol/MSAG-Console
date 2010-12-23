package mobi.eyeline.informer.web.config;

/**
 * @author Artem Snopkov
 */
public class ConfigException extends Exception {
  ConfigException(String message) {
    super(message);
  }

  ConfigException(Throwable cause) {
    super(cause);
  }
}
