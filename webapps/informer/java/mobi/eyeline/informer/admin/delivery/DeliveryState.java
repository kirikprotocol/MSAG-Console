package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryState {

  private static final ValidationHelper vh = new ValidationHelper(DeliveryState.class);

  private Date date;

  private DeliveryStatus status;

  public Date getDate() {
    return date;
  }

  public void setDate(Date date) {
    this.date = date;
  }

  public DeliveryStatus getStatus() {
    return status;
  }

  public void setStatus(DeliveryStatus status) throws AdminException{
    vh.checkNotNull("status", status);
    this.status = status;
  }

  public DeliveryState cloneState() {
    DeliveryState state = new DeliveryState();
    state.date = date;
    state.status = status;
    return state;
  }
}
