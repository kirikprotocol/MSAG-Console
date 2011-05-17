package mobi.eyeline.informer.admin.delivery;

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

  protected Integer archiveTime;
  protected Time messageTimeToLive;

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

  public void setSourceAddress(Address sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public Integer getArchiveTime() {
    return archiveTime;
  }

  public void setArchiveTime(Integer archiveTime) {
    this.archiveTime = archiveTime;
  }

  public String getSingleText() {
    return singleText;
  }

  public void setSingleText(String singleText) {
    this.singleText = singleText;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
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

  public void setStartDate(Date startDate) {
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

  public void setActivePeriodEnd(Time activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
  }

  public Time getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Time activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
  }

  public Day[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public void setActiveWeekDays(Day[] days) {
    this.activeWeekDays = days;
  }

  public Time getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(Time validityPeriod) {
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

  public void setDeliveryMode(DeliveryMode deliveryMode) {
    this.deliveryMode = deliveryMode;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
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

  public Time getMessageTimeToLive() {
    return messageTimeToLive;
  }

  public void setMessageTimeToLive(Time messageTimeToLive) {
    this.messageTimeToLive = messageTimeToLive;
  }
}
