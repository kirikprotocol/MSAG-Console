package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryException extends AdminException {

  private ErrorStatus errorStatus = ErrorStatus.Unknown;

  public static enum ErrorStatus {
    NotAuthorized(1),
    CommandHandling(2),
    AdminRoleRequired(3),
    TooManyDeliveries(4),
    AuthFailed(5),
    RequestNotFound(6),
    Expired(7),
    NoSuchEntry(8),
    Unknown(9);

    private static Map<Integer, ErrorStatus> errors = new HashMap<Integer, ErrorStatus>(10) {{
      for(ErrorStatus s : ErrorStatus.values()) {
        put(s.errorCode, s);
      }
    }};

    private int errorCode;

    ErrorStatus(int errorCode) {
      this.errorCode = errorCode;
    }

    public static ErrorStatus valueOf(int code) {
      return errors.get(code);
    }

    public int getErrorCode() {
      return errorCode;
    }
  }


  public DeliveryException(ErrorStatus errorStatus, String message) {
    super("error."+errorStatus.toString(), new String[]{message});
    this.errorStatus = errorStatus;
  }

  public DeliveryException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  public DeliveryException(String key, Throwable cause) {
    super(key, cause);
  }

  public DeliveryException(String key, String... args) {
    super(key, args);
  }

  public void setErrorStatus(ErrorStatus errorStatus) {
    this.errorStatus = errorStatus;
  }

  public ErrorStatus getErrorStatus() {
    return errorStatus;
  }

  public String getKey() {
    return key;
  }

  public String[] getArgs() {
    return args;
  }
}