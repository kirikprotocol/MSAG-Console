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
  private MessageFields[] fields;

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) throws AdminException{
    vh.checkNotNull("deliveryId", deliveryId);
    this.deliveryId = deliveryId;
  }

  public MessageState[] getStates() {
    return states;
  }

  public void setStates(MessageState[] states) {
    this.states = states;
  }

  public String[] getMsisdnFilter() {
    return msisdnFilter;
  }

  public void setMsisdnFilter(String[] msisdnFilter) {
    this.msisdnFilter = msisdnFilter;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) throws AdminException {
    vh.checkNotNull("startDate", startDate);
    if (endDate != null) {
      vh.checkGreaterThan("startDate", endDate, startDate);
    }
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) throws AdminException {
    vh.checkNotNull("endDate", endDate);
    if (startDate != null) {
      vh.checkGreaterThan("endDate", endDate, startDate);
    }
    this.endDate = endDate;
  }

  public MessageFields[] getFields() {
    return fields;
  }

  public void setFields(MessageFields[] fields) {
    this.fields = fields;
  }
}
