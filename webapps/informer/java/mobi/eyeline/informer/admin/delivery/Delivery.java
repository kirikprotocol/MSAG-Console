package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

/**
 * Настройки рассылки
 *
 * @author Aleksandr Khalitov
 */
public class Delivery {

  private static final ValidationHelper vh = new ValidationHelper(Delivery.class);

  private Integer id;

  private String name;
  private String userId;
  private int priority;
  private boolean transactionMode;

  private Date startDate;
  private Date endDate;
  private Date activePeriodEnd;
  private Date activePeriodStart;
  private Day[] activeWeekDays;
  private Date validityDate;
  private String validityPeriod;

  private boolean flash;
  private boolean secret;
  private boolean secretFlash;
  private String secretMessage;
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

  public void setName(String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public String getUserId() {
    return userId;
  }

  public void setUserId(String userId) {
    this.userId = userId;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) throws AdminException {
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

  public Date getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(Date activePeriodEnd) throws AdminException {
    vh.checkNotNull("activePeriodEnd", activePeriodEnd);
    this.activePeriodEnd = activePeriodEnd;
  }

  public Date getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Date activePeriodStart) throws AdminException {
    vh.checkNotNull("activePeriodStart", activePeriodStart);
    this.activePeriodStart = activePeriodStart;
  }

  public Day[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public void setActiveWeekDays(Day[] days) throws AdminException {
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

  public String getSecretMessage() {
    return secretMessage;
  }

  public void setSecretMessage(String secretMessage) {
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

  public void setDeliveryMode(DeliveryMode deliveryMode) throws AdminException {
    vh.checkNotNull("deliveryMode", deliveryMode);
    this.deliveryMode = deliveryMode;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) throws AdminException {
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

    private static final Map<Integer, Day> days = new HashMap<Integer, Day>(7);

    static {
      for (Day d : values()) {
        days.put(d.getDay(), d);
      }
    }

    private final int day;

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

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Delivery delivery = (Delivery) o;

    SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    if (flash != delivery.flash) return false;
    if (priority != delivery.priority) return false;
    if (replaceMessage != delivery.replaceMessage) return false;
    if (retryOnFail != delivery.retryOnFail) return false;
    if (secret != delivery.secret) return false;
    if (secretFlash != delivery.secretFlash) return false;
    if (secretMessage != delivery.secretMessage) return false;
    if (transactionMode != delivery.transactionMode) return false;
    if (useDataSm != delivery.useDataSm) return false;
    if (activePeriodEnd != null ? !timeFormat.format(activePeriodEnd).equals(delivery.activePeriodEnd == null ? null : timeFormat.format(delivery.activePeriodEnd)) : delivery.activePeriodEnd != null)
      return false;
    if (activePeriodStart != null ? !dateFormat.format(activePeriodStart).equals(delivery.activePeriodStart == null ? null : dateFormat.format(delivery.activePeriodStart)) : delivery.activePeriodStart != null)
      return false;
    if (!Arrays.equals(activeWeekDays, delivery.activeWeekDays)) return false;
    if (deliveryMode != delivery.deliveryMode) return false;
    if (endDate != null ? !dateFormat.format(endDate).equals(delivery.endDate == null ? null : dateFormat.format(delivery.endDate)) : delivery.endDate != null)
      return false;
    if (id != null ? !id.equals(delivery.id) : delivery.id != null) return false;
    if (name != null ? !name.equals(delivery.name) : delivery.name != null) return false;
    if (owner != null ? !owner.equals(delivery.owner) : delivery.owner != null) return false;
    if (retryPolicy != null ? !retryPolicy.equals(delivery.retryPolicy) : delivery.retryPolicy != null) return false;
    if (startDate != null ? !dateFormat.format(startDate).equals(delivery.startDate == null ? null : dateFormat.format(delivery.startDate)) : delivery.startDate != null)
      return false;
    if (svcType != null ? !svcType.equals(delivery.svcType) : delivery.svcType != null) return false;
    if (userId != null ? !userId.equals(delivery.userId) : delivery.userId != null) return false;
    if (validityDate != null ? !dateFormat.format(validityDate).equals(delivery.validityDate == null ? null : dateFormat.format(delivery.validityDate)) : delivery.validityDate != null)
      return false;
    if (validityPeriod != null ? !validityPeriod.equals(delivery.validityPeriod) : delivery.validityPeriod != null)
      return false;

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }

  public Delivery cloneDelivery() {
    Delivery d = new Delivery();
    d.id = id;

    d.name = name;
    d.userId = userId;
    d.priority = priority;
    d.transactionMode = transactionMode;

    d.startDate = startDate == null ? null : new Date(startDate.getTime());
    d.endDate = endDate == null ? null : new Date(endDate.getTime());
    d.activePeriodEnd = activePeriodEnd == null ? null : new Date(activePeriodEnd.getTime());
    d.activePeriodStart = activePeriodStart == null ? null : new Date(activePeriodStart.getTime());
    d.activeWeekDays = new Day[activeWeekDays.length];
    System.arraycopy(activeWeekDays, 0, d.activeWeekDays, 0, activeWeekDays.length);
    d.validityDate = validityDate == null ? null : new Date(validityDate.getTime());
    d.validityPeriod = validityPeriod;

    d.flash = flash;
    d.secret = secret;
    d.secretFlash = secretFlash;
    d.secretMessage = secretMessage;
    d.useDataSm = useDataSm;
    d.deliveryMode = deliveryMode;

    d.owner = owner;

    d.retryOnFail = retryOnFail;
    d.retryPolicy = retryPolicy;

    d.replaceMessage = replaceMessage;
    d.svcType = svcType;
    return d;
  }
}
