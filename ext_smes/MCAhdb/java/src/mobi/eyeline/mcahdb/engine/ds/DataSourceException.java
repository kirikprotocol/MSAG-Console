package mobi.eyeline.mcahdb.engine.ds;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class DataSourceException extends Exception {
  public DataSourceException() {
  }

  public DataSourceException(String message) {
    super(message);
  }

  public DataSourceException(String message, Throwable cause) {
    super(message, cause);
  }

  public DataSourceException(Throwable cause) {
    super(cause);
  }
}
