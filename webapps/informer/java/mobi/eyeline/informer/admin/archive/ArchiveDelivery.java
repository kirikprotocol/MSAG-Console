package mobi.eyeline.informer.admin.archive;


import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.delivery.DeliveryStatusHistory;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveDelivery {

  private Integer id;

  private String name;
  private int priority;
  private boolean transactionMode;

  private Date startDate;
  private Date endDate;
  private Time activePeriodEnd;
  private Time activePeriodStart;
  private Day[] activeWeekDays;
  private Time validityPeriod;

  private boolean flash;

  private String owner;

  private boolean retryOnFail;
  private String retryPolicy;


  private Address sourceAddress;

  private String emailNotification;

  private Address smsNotification;

  private boolean useDataSm;

  private DeliveryMode deliveryMode;

  private List<DeliveryStatusHistory.Item> history;

  private int delivered;

  private int failed;

  ArchiveDelivery() {
  }

  public int getDelivered() {
    return delivered;
  }

  void setDelivered(int delivered) {
    this.delivered = delivered;
  }

  public int getFailed() {
    return failed;
  }

  void setFailed(int failed) {
    this.failed = failed;
  }

  public List<DeliveryStatusHistory.Item> getHistory() {
    return history == null ? null : new ArrayList<DeliveryStatusHistory.Item>(history);
  }

  void setHistory(List<DeliveryStatusHistory.Item> history) {
    this.history = history;
  }

  public boolean isUseDataSm() {
    return useDataSm;
  }

  void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
  }

  public DeliveryMode getDeliveryMode() {
    return deliveryMode;
  }

  void setDeliveryMode(DeliveryMode deliveryMode) {
    this.deliveryMode = deliveryMode;
  }

  public String getEmailNotification() {
    return emailNotification;
  }

  void setEmailNotification(String emailNotification) {
    this.emailNotification = emailNotification;
  }

  public Address getSmsNotification() {
    return smsNotification;
  }

  void setSmsNotification(Address smsNotification) {
    this.smsNotification = smsNotification;
  }

  public Integer getId() {
    return id;
  }

  void setId(Integer id) {
    this.id = id;
  }

  public String getName() {
    return name;
  }

  void setName(String name) {
    this.name = name;
  }

  public int getPriority() {
    return priority;
  }

  void setPriority(int priority) {
    this.priority = priority;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public Date getStartDate() {
    return startDate;
  }

  void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public Time getActivePeriodEnd() {
    return activePeriodEnd;
  }

  void setActivePeriodEnd(Time activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
  }

  public Time getActivePeriodStart() {
    return activePeriodStart;
  }

  void setActivePeriodStart(Time activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
  }

  public Day[] getActiveWeekDays() {
    return activeWeekDays;
  }

  void setActiveWeekDays(Day[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

  public Time getValidityPeriod() {
    return validityPeriod;
  }

  void setValidityPeriod(Time validityPeriod) {
    this.validityPeriod = validityPeriod;
  }

  public boolean isFlash() {
    return flash;
  }

  void setFlash(boolean flash) {
    this.flash = flash;
  }

  public String getOwner() {
    return owner;
  }

  void setOwner(String owner) {
    this.owner = owner;
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public String getRetryPolicy() {
    return retryPolicy;
  }

  void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
  }

  public Address getSourceAddress() {
    return sourceAddress;
  }

  void setSourceAddress(Address sourceAddress) {
    this.sourceAddress = sourceAddress;
  }
}
