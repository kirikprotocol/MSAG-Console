package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * Прототип для создания рассылки
 * @author Artem Snopkov
 */
public class DeliveryPrototype {

  private static final ValidationHelper vh = new ValidationHelper(Delivery.class);
  
  protected String name;
  protected int priority;
  protected boolean transactionMode;

  protected Date startDate;
  protected Date endDate;
  protected Time activePeriodEnd;
  protected Time activePeriodStart;
  protected Day[] activeWeekDays;
  protected Time validityPeriod;

  protected boolean flash;
  protected boolean useDataSm;
  protected DeliveryMode deliveryMode;

  protected String owner;

  protected boolean retryOnFail;
  protected String retryPolicy;

  protected boolean replaceMessage;
  protected String svcType;

  protected Address sourceAddress;

  protected String singleText;

  protected final Properties properties = new Properties();

  protected boolean enableMsgFinalizationLogging;
  protected boolean enableStateChangeLogging;

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

  public Address getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(Address sourceAddress) throws AdminException {
    vh.checkNotNull("sourceAddress", sourceAddress);
    this.sourceAddress = sourceAddress;
  }

  public String getSingleText() {
    return singleText;
  }

  public void setSingleText(String singleText) throws AdminException {
    vh.checkNotEmpty("singleText", singleText);
    this.singleText = singleText;
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
    return activeWeekDays;
  }

  public void setActiveWeekDays(Day[] days) throws AdminException {
    vh.checkSizeGreaterThen("activeWeekDays", days, 0);
    this.activeWeekDays = days;
  }

  public Time getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(Time validityPeriod) throws AdminException{
    if (validityPeriod != null)
      vh.checkPositive("validityPeriod", validityPeriod.getHour());
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

  public boolean isEnableMsgFinalizationLogging() {
    return enableMsgFinalizationLogging;
  }

  public void setEnableMsgFinalizationLogging(boolean enableMsgFinalizationLogging) {
    this.enableMsgFinalizationLogging = enableMsgFinalizationLogging;
  }

  public boolean isEnableStateChangeLogging() {
    return enableStateChangeLogging;
  }

  public void setEnableStateChangeLogging(boolean enableStateChangeLogging) {
    this.enableStateChangeLogging = enableStateChangeLogging;
  }
}
