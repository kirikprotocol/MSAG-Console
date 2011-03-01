package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryException extends AdminException {

  private ErrorStatus errorStatus = ErrorStatus.Unknown;

  public static enum ErrorStatus {
    NotAuthorized,
    CommandHandling,
    AdminRoleRequired,
    TooManyDeliveries,
    AuthFailed,
    RequestNotFound,
    Expired,
    NoSuchEntry,
    Unknown
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
}