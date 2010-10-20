package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

/**
 * Настройки рассылки
 * @author Aleksandr Khalitov
 */
public class Delivery {

  private static final ValidationHelper vh = new ValidationHelper(Delivery.class);

  private Integer id;

  private String name;
  private String userId;
  private boolean enabled;
  private int priority;
  private boolean transactionMode;

  private Date startDate;
  private Date endDate;
  private String activePeriodEnd;
  private String activePeriodStart;
  private Day[] activeWeekDays;
  private Date validityDate;
  private String validityPeriod;

  private boolean flash;
  private boolean secret;
  private boolean secretFlash;
  private boolean secretMessage;
  private boolean useDataSm;
  private DeliveryMode deliveryMode;

  private String owner;

  private boolean retryOnFail;
  private String retryPolicy;

  private boolean replaceMessage;
  private String svcType;

  public Integer getId() {
    return id;
  }

  void setId(int id) {
    this.id = id;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) throws AdminException{
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public String getUserId() {
    return userId;
  }

  public void setUserId(String userId) {
    this.userId = userId;
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) throws AdminException{
    vh.checkBetween("priority", priority, 1, 1000);
    this.priority = priority;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) throws AdminException {
    if(startDate != null && endDate != null) {
      vh.checkGreaterThan("startDate", endDate, startDate);
    }
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) throws AdminException{
    if(startDate != null && endDate != null) {
      vh.checkGreaterThan("endDate", endDate, startDate);
    }
    this.endDate = endDate;
  }

  public String getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(String activePeriodEnd) throws AdminException {
    if(activePeriodStart != null && activePeriodEnd != null) {
      vh.checkGreaterThan("activePeriodEnd", activePeriodEnd, activePeriodEnd);
    }
    this.activePeriodEnd = activePeriodEnd;
  }

  public String getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(String activePeriodStart) throws AdminException{
    if(activePeriodStart != null && activePeriodEnd != null) {
      vh.checkGreaterThan("activePeriodStart", activePeriodEnd, activePeriodEnd);
    }
    this.activePeriodStart = activePeriodStart;
  }

  public Day[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public void setActiveWeekDays(Day[] days) throws AdminException{
    vh.checkSizeGreaterThen("activeWeekDays", days, 0);
    this.activeWeekDays = days;
  }

  public Date getValidityDate() {
    return validityDate;
  }

  public void setValidityDate(Date validityDate) {
    this.validityDate = validityDate;
  }

  public String getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(String validityPeriod) {
    this.validityPeriod = validityPeriod;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
  }

  public boolean isSecret() {
    return secret;
  }

  public void setSecret(boolean secret) {
    this.secret = secret;
  }

  public boolean isSecretFlash() {
    return secretFlash;
  }

  public void setSecretFlash(boolean secretFlash) {
    this.secretFlash = secretFlash;
  }

  public boolean isSecretMessage() {
    return secretMessage;
  }

  public void setSecretMessage(boolean secretMessage) {
    this.secretMessage = secretMessage;
  }

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
  }

  public DeliveryMode getDeliveryMode() {
    return deliveryMode;
  }

  public void setDeliveryMode(DeliveryMode deliveryMode) throws AdminException{
    vh.checkNotNull("deliveryMode", deliveryMode);
    this.deliveryMode = deliveryMode;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) throws AdminException{
    vh.checkNotEmpty("owner", owner);
    this.owner = owner;
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public String getRetryPolicy() {
    return retryPolicy;
  }

  public void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
  }

  public boolean isReplaceMessage() {
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage) {
    this.replaceMessage = replaceMessage;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }

  /**
   * Дни недели
   */
  public static enum Day {
    Mon(1),
    Tue(2),
    Wed(3),
    Thu(4),
    Fri(5),
    Sat(6),
    Sun(7);

    private static final Map<Integer,Day> days = new HashMap<Integer, Day>(7);
    static {
      for(Day d : values()) {
        days.put(d.getDay(), d);
      }
    }

    private int day;

    Day(int day) {
      this.day = day;
    }
    public int getDay() {
      return day;
    }
    public static Day valueOf(int d) {
      return days.get(d);
    }
  }
}
