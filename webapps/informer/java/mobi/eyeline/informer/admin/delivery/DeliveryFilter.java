package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Фильтр извлечения рассылок
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryFilter {


  private String[] userIdFilter;
  private String[] nameFilter;
  private DeliveryStatus[] statusFilter;
  private Date startDateFrom;
  private Date startDateTo;
  private Date createDateFrom;
  private Date createDateTo;
  private Date endDateFrom;
  private Date endDateTo;

  public String[] getUserIdFilter() {
    return userIdFilter;
  }

  public void setUserIdFilter(String... userIdFilter) {
    this.userIdFilter = userIdFilter;
  }

  public String[] getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String... nameFilter) {
    this.nameFilter = nameFilter;
  }

  public DeliveryStatus[] getStatusFilter() {
    return statusFilter;
  }

  public void setStatusFilter(DeliveryStatus... statusFilter) {
    this.statusFilter = statusFilter;
  }

  public Date getStartDateFrom() {
    return startDateFrom;
  }

  public void setStartDateFrom(Date startDateFrom) {
    this.startDateFrom = startDateFrom;
  }

  public Date getCreateDateFrom() {
    return createDateFrom;
  }

  public void setCreateDateFrom(Date createDateFrom) {
    this.createDateFrom = createDateFrom;
  }

  public Date getCreateDateTo() {
    return createDateTo;
  }

  public void setCreateDateTo(Date createDateTo) {
    this.createDateTo = createDateTo;
  }

  public Date getStartDateTo() {
    return startDateTo;
  }

  public void setStartDateTo(Date startDateTo) {
    this.startDateTo = startDateTo;
  }

  public Date getEndDateFrom() {
    return endDateFrom;
  }

  public void setEndDateFrom(Date endDateFrom) {
    this.endDateFrom = endDateFrom;
  }

  public Date getEndDateTo() {
    return endDateTo;
  }

  public void setEndDateTo(Date endDateTo) {
    this.endDateTo = endDateTo;
  }

}
