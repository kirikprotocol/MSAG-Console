package mobi.eyeline.informer.util.config;

/**
 * User: artem
 * Date: 16.02.2007
 */

public final class XmlConfigException extends Exception {

  XmlConfigException() {
  }

  XmlConfigException(String message) {
    super(message);
  }

  XmlConfigException(Throwable cause) {
    super(cause);
  }

  XmlConfigException(String message, Throwable cause) {
    super(message, cause);
  }
}
