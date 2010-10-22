package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.text.SimpleDateFormat;
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

  public void setStatus(DeliveryStatus status) throws AdminException {
    vh.checkNotNull("status", status);
    this.status = status;
  }

  public DeliveryState cloneState() {
    DeliveryState state = new DeliveryState();
    state.date = date;
    state.status = status;
    return state;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    DeliveryState state = (DeliveryState) o;
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    if (date != null ? !dateFormat.format(date).equals(state.date == null ? null : dateFormat.format(state.date)) : state.date != null) return false;
    if (status != state.status) return false;

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }
}
