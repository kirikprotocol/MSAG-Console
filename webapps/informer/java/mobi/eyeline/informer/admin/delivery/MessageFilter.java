package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.Date;

/**
 * Фильтр для извлечения сообщений
 *
 * @author Aleksandr Khalitov
 */
public class MessageFilter {

  private static final ValidationHelper vh = new ValidationHelper(MessageFilter.class);

  private Integer deliveryId;
  private MessageState[] states;
  private String[] msisdnFilter;
  private Date startDate;
  private Date endDate;
  private int[] errorCodes;

  private boolean noResended;

  public MessageFilter(Integer deliveryId, Date startDate, Date endDate) {
    if (deliveryId == null || startDate == null || endDate == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    this.deliveryId = deliveryId;
    this.startDate = startDate;
    this.endDate = endDate;
  }

  public MessageFilter(MessageFilter copy) {
    this.deliveryId = copy.deliveryId;
    this.states = copy.states;
    this.msisdnFilter = copy.msisdnFilter;
    this.startDate = copy.startDate;
    this.endDate = copy.endDate;
    this.errorCodes = copy.errorCodes;
    this.noResended = copy.noResended;
  }

  public boolean isNoResended() {
    return noResended;
  }

  public void setNoResended(boolean noResended) {
    this.noResended = noResended;
  }

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) throws AdminException {
    vh.checkNotNull("deliveryId", deliveryId);
    this.deliveryId = deliveryId;
  }

  public MessageState[] getStates() {
    return states;
  }

  public void setStates(MessageState... states) {
    this.states = states;
  }

  public String[] getMsisdnFilter() {
    return msisdnFilter;
  }

  public void setMsisdnFilter(String... msisdnFilter) {
    this.msisdnFilter = msisdnFilter;
  }

  public int[] getErrorCodes() {
    return errorCodes;
  }

  public void setErrorCodes(int... errorCodes) {
    this.errorCodes = errorCodes;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) throws AdminException {
    vh.checkNotNull("startDate", startDate);
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) throws AdminException {
    vh.checkNotNull("endDate", endDate);
    this.endDate = endDate;
  }

}
