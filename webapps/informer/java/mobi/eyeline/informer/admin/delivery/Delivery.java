package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Настройки рассылки
 *
 * @author Aleksandr Khalitov
 */
public class Delivery implements Serializable {

  private static final ValidationHelper vh = new ValidationHelper(Delivery.class);

  public static enum Type {
    SingleText, Common;

    public String getValue() {
      return toString();
    }
  }

  private Integer id;

  private String name;
  private int priority;
  private boolean transactionMode;

  private Date startDate;
  private Date endDate;
  private Date activePeriodEnd;
  private Date activePeriodStart;
  private Day[] activeWeekDays;
  private String validityPeriod;

  private boolean flash;
  private boolean useDataSm;
  private DeliveryMode deliveryMode;

  private String owner;

  private boolean retryOnFail;
  private String retryPolicy;

  private boolean replaceMessage;
  private String svcType;


  private Address sourceAddress;

  private String singleText;

  private final Type type;

  private final Properties properties = new Properties();

  public static Delivery newSingleTextDelivery() {
    return new Delivery(Type.SingleText);
  }

  public static Delivery newCommonDelivery() {
    return new Delivery(Type.Common);
  }


  public Delivery(Type type) {
    this.type = type;
  }

  public void setProperty(String name, String value) {
    properties.setProperty(name, value);
  }

  public String removeProperty(String name) {
    return (String) properties.remove(name);
  }

  public void addProperties(Map<String, String> props) {
    properties.putAll(props);
  }

  public String getProperty(String name) {
    return properties.getProperty(name);
  }

  public boolean containsProperty(String name) {
    return properties.containsKey(name);
  }

  public Properties getProperties() {
    Properties properties = new Properties();
    properties.putAll(this.properties);
    return properties;
  }

  public Type getType() {
    return type;
  }

  public Address getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(Address sourceAddress) throws AdminException {
    vh.checkNotNull("sourceAddress", sourceAddress);
    this.sourceAddress = sourceAddress;
  }

//  public String getEmailNotificationAddress() {
//    return emailNotificationAddress;
//  }
//
//  public void setEmailNotificationAddress(String emailNotificationAddress) {
//    this.emailNotificationAddress = emailNotificationAddress;
//  }
//
//  public Address getSmsNotificationAddress() {
//    return smsNotificationAddress;
//  }
//
//  public void setSmsNotificationAddress(Address smsNotificationAddress) {
//    this.smsNotificationAddress = smsNotificationAddress;
//  }

  public String getSingleText() {
    return singleText;
  }

  public void setSingleText(String singleText) throws AdminException {
    if (type == Type.Common) {
      throw new DeliveryException("illegal_delivery_type");
    }
    vh.checkNotEmpty("singleText", singleText);
    this.singleText = singleText;
  }

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
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
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

  public String getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(String validityPeriod) throws AdminException {
    vh.checkNotEmpty("validityPeriod", validityPeriod);
    this.validityPeriod = validityPeriod;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
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

  @SuppressWarnings({"RedundantIfStatement"})
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
    if (validityPeriod != null ? !validityPeriod.equals(delivery.validityPeriod) : delivery.validityPeriod != null)
      return false;
    if (singleText != null ? !singleText.equals(delivery.singleText) : delivery.singleText != null)
      return false;
    if (type != delivery.type) {
      return false;
    }
    if (sourceAddress != null ? !sourceAddress.getSimpleAddress().equals(delivery.sourceAddress.getSimpleAddress()) : delivery.sourceAddress != null)
      return false;

    if (properties.size() != delivery.properties.size()) {
      return false;
    }
    for (Map.Entry e : properties.entrySet()) {
      if (!delivery.properties.containsKey(e.getKey())) {
        return false;
      }
      if (!delivery.properties.get(e.getKey()).equals(e.getValue())) {
        return false;
      }
    }

    return true;
  }

  public Delivery cloneDelivery() {
    Delivery d = new Delivery(type);
    d.id = id;

    d.name = name;
    d.priority = priority;
    d.transactionMode = transactionMode;

    d.startDate = startDate == null ? null : new Date(startDate.getTime());
    d.endDate = endDate == null ? null : new Date(endDate.getTime());
    d.activePeriodEnd = activePeriodEnd == null ? null : new Date(activePeriodEnd.getTime());
    d.activePeriodStart = activePeriodStart == null ? null : new Date(activePeriodStart.getTime());
    d.activeWeekDays = new Day[activeWeekDays.length];
    System.arraycopy(activeWeekDays, 0, d.activeWeekDays, 0, activeWeekDays.length);
    d.validityPeriod = validityPeriod;

    d.flash = flash;
    d.useDataSm = useDataSm;
    d.deliveryMode = deliveryMode;

    d.owner = owner;

    d.retryOnFail = retryOnFail;
    d.retryPolicy = retryPolicy;

    d.replaceMessage = replaceMessage;
    d.svcType = svcType;

    d.sourceAddress = sourceAddress == null ? null : new Address(sourceAddress.getSimpleAddress());
    d.singleText = singleText;
    d.properties.putAll(properties);
    return d;
  }
}
