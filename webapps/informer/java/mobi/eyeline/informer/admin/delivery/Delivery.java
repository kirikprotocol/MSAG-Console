package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Настройки рассылки
 *
 * @author Aleksandr Khalitov
 */
public class Delivery implements Serializable {

  private static final Logger logger = Logger.getLogger(Delivery.class);

  private static final ValidationHelper vh = new ValidationHelper(Delivery.class);

  public static enum Type {
    /**
     * Рассылка с одним текстом
     */
    SingleText,
    /**
     * Рассылка с индивидуальными текстами для каждого реципиента
     */
    IndividualTexts;

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
  private Time activePeriodEnd;
  private Time activePeriodStart;
  private Day[] activeWeekDays;
  private Time validityPeriod;

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

  private Type type;

  private final Properties properties = new Properties();

  private boolean enableMsgFinalizationLogging;
  private boolean enableStateChangeLogging;

  private DeliveryStatus status;

  private boolean loaded;
  private DeliveryManager m;
  private String login;
  private String password;

  Delivery() {
  }



  void setDeliveryManager(DeliveryManager m) {
    this.m = m;
  }

  void setLogin(String login) {
    this.login = login;
  }

  void setPassword(String password) {
    this.password = password;
  }

  private void loadDelivery() {
    if (m != null && !loaded) {
      Delivery d = null;
      try {
        d = m.getDelivery(login, password, id);
        copyFrom(d);
      } catch (AdminException e) {
        logger.error(e);
      }
      loaded = true;
    }
  }

  public void setLoaded(boolean loaded) {
    this.loaded = loaded;
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

  void setType(Type type) {
    this.type = type;
  }


  public Address getSourceAddress() {
    loadDelivery();
    return sourceAddress;
  }

  public void setSourceAddress(Address sourceAddress) throws AdminException {
    vh.checkNotNull("sourceAddress", sourceAddress);
    this.sourceAddress = sourceAddress;
  }

  public String getSingleText() {
      loadDelivery();
    return singleText;
  }

  public void setSingleText(String singleText) throws AdminException {
    if (type == Type.IndividualTexts) {
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
    loadDelivery();
    return priority;
  }

  public void setPriority(int priority) throws AdminException {
    vh.checkBetween("priority", priority, 1, 1000);
    this.priority = priority;
  }

  public boolean isTransactionMode() {
    loadDelivery();
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

  public Time getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(Time activePeriodEnd) throws AdminException {
    vh.checkNotNull("activePeriodEnd", activePeriodEnd);
    this.activePeriodEnd = activePeriodEnd;
  }

  public Time getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Time activePeriodStart) throws AdminException {
    vh.checkNotNull("activePeriodStart", activePeriodStart);
    this.activePeriodStart = activePeriodStart;
  }

  public Day[] getActiveWeekDays() {
    loadDelivery();
    return activeWeekDays;
  }

  public void setActiveWeekDays(Day[] days) throws AdminException {
    vh.checkSizeGreaterThen("activeWeekDays", days, 0);
    this.activeWeekDays = days;
  }

  public Time getValidityPeriod() {
    loadDelivery();
    return validityPeriod;
  }

  public void setValidityPeriod(Time validityPeriod) throws AdminException{
    if (validityPeriod != null)
      vh.checkPositive("validityPeriod", validityPeriod.getHour());
    this.validityPeriod = validityPeriod;
  }

  public boolean isFlash() {
    loadDelivery();
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
  }

  public boolean isUseDataSm() {
    loadDelivery();
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
  }

  public DeliveryMode getDeliveryMode() {
    loadDelivery();
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
    loadDelivery();
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public String getRetryPolicy() {
    loadDelivery();
    return retryPolicy;
  }

  public void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
  }

  public boolean isReplaceMessage() {
    loadDelivery();
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage) {
    this.replaceMessage = replaceMessage;
  }

  public String getSvcType() {
    loadDelivery();
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }

  public boolean isEnableMsgFinalizationLogging() {
    loadDelivery();
    return enableMsgFinalizationLogging;
  }

  public void setEnableMsgFinalizationLogging(boolean enableMsgFinalizationLogging) {
    this.enableMsgFinalizationLogging = enableMsgFinalizationLogging;
  }

  public boolean isEnableStateChangeLogging() {
    loadDelivery();
    return enableStateChangeLogging;
  }

  public void setEnableStateChangeLogging(boolean enableStateChangeLogging) {
    this.enableStateChangeLogging = enableStateChangeLogging;
  }

  public DeliveryStatus getStatus() {
    if (status == null && m != null) {
      try {
        DeliveryStatusHistory h = m.getDeliveryStatusHistory(login, password, id);
        status = h.getHistoryItems().get(h.getHistoryItems().size() - 1).getStatus();
      } catch (AdminException e) {
        logger.error(e,e);
      }
    }
    return status;
  }

  void setStatus(DeliveryStatus status) {
    this.status = status;
  }

  @SuppressWarnings({"RedundantIfStatement"})
  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Delivery delivery = (Delivery) o;

    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    if (flash != delivery.flash) return false;
    if (priority != delivery.priority) return false;
    if (replaceMessage != delivery.replaceMessage) return false;
    if (retryOnFail != delivery.retryOnFail) return false;
    if (transactionMode != delivery.transactionMode) return false;
    if (useDataSm != delivery.useDataSm) return false;
    if (!activePeriodEnd.equals(delivery.activePeriodEnd)) return false;
    if (!activePeriodStart.equals(delivery.activePeriodStart)) return false;
    
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
    if (enableMsgFinalizationLogging != delivery.enableMsgFinalizationLogging)
      return false;
    if (enableStateChangeLogging != delivery.enableStateChangeLogging)
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
    Delivery d = new Delivery();
    d.type = type;

    d.id = id;

    d.m = m;
    d.login = login;
    d.password = password;
    d.status = status;

    d.name = name;
    d.priority = priority;
    d.transactionMode = transactionMode;

    d.startDate = startDate == null ? null : new Date(startDate.getTime());
    d.endDate = endDate == null ? null : new Date(endDate.getTime());
    d.activePeriodEnd = activePeriodEnd == null ? null : new Time(activePeriodEnd);
    d.activePeriodStart = activePeriodStart == null ? null : new Time(activePeriodStart);
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
    d.enableMsgFinalizationLogging = enableMsgFinalizationLogging;
    d.enableStateChangeLogging = enableStateChangeLogging;
    d.properties.putAll(properties);
    return d;
  }

  void copyFrom(Delivery d) {
    m = d.m;
    login = d.login;
    password = d.password;

    name = d.name;
    priority = d.priority;
    transactionMode = d.transactionMode;

    startDate = d.startDate == null ? null : new Date(d.startDate.getTime());
    endDate = d.endDate == null ? null : new Date(d.endDate.getTime());
    activePeriodEnd = d.activePeriodEnd == null ? null : new Time(d.activePeriodEnd);
    activePeriodStart = d.activePeriodStart == null ? null : new Time(d.activePeriodStart);
    activeWeekDays = new Day[d.activeWeekDays.length];
    System.arraycopy(d.activeWeekDays, 0, activeWeekDays, 0, d.activeWeekDays.length);
    validityPeriod = d.validityPeriod;

    flash = d.flash;
    useDataSm = d.useDataSm;
    deliveryMode = d.deliveryMode;

    owner = d.owner;

    retryOnFail = d.retryOnFail;
    retryPolicy = d.retryPolicy;

    replaceMessage = d.replaceMessage;
    svcType = d.svcType;

    sourceAddress = d.sourceAddress == null ? null : new Address(d.sourceAddress.getSimpleAddress());
    singleText = d.singleText;
    enableMsgFinalizationLogging = d.enableMsgFinalizationLogging;
    enableStateChangeLogging = d.enableStateChangeLogging;
    properties.putAll(d.properties);
  }
  
  void copyFrom(DeliveryPrototype d) {

    name = d.name;
    priority = d.priority;
    transactionMode = d.transactionMode;

    startDate = d.startDate == null ? null : new Date(d.startDate.getTime());
    endDate = d.endDate == null ? null : new Date(d.endDate.getTime());
    activePeriodEnd = d.activePeriodEnd == null ? null : new Time(d.activePeriodEnd);
    activePeriodStart = d.activePeriodStart == null ? null : new Time(d.activePeriodStart);
    activeWeekDays = new Day[d.activeWeekDays.length];
    System.arraycopy(d.activeWeekDays, 0, activeWeekDays, 0, d.activeWeekDays.length);
    validityPeriod = d.validityPeriod;

    flash = d.flash;
    useDataSm = d.useDataSm;
    deliveryMode = d.deliveryMode;

    owner = d.owner;

    retryOnFail = d.retryOnFail;
    retryPolicy = d.retryPolicy;

    replaceMessage = d.replaceMessage;
    svcType = d.svcType;

    sourceAddress = d.sourceAddress == null ? null : new Address(d.sourceAddress.getSimpleAddress());
    singleText = d.singleText;
    enableMsgFinalizationLogging = d.enableMsgFinalizationLogging;
    enableStateChangeLogging = d.enableStateChangeLogging;
    properties.putAll(d.properties);
  }
}
