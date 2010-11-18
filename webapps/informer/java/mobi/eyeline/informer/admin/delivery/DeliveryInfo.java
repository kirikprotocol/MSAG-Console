package mobi.eyeline.informer.admin.delivery;

import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * Краткая информация о рассылке
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryInfo {

  private int deliveryId;
  private String userId;
  private String name;
  private DeliveryStatus status;
  private Date startDate;
  private Date endDate;
  private Date activityPeriodStart;
  private Date activityPeriodEnd;

  private final Properties properties = new Properties();


  public void setProperty(String name, String value) {
    properties.setProperty(name, value);
  }

  public void addProperties(Map<String, String> props) {
    properties.putAll(props);
  }

  public String getProperty(String name) {
    return properties.getProperty(name);
  }

  public String removeProperty(String name) {
    return (String) properties.remove(name);
  }

  public boolean containsProperty(String name) {
    return properties.containsKey(name);
  }

  public Properties getProperties() {
    Properties properties = new Properties();
    properties.putAll(this.properties);
    return properties;
  }


  public int getDeliveryId() {
    return deliveryId;
  }

  void setDeliveryId(int deliveryId) {
    this.deliveryId = deliveryId;
  }

  public String getUserId() {
    return userId;
  }

  void setUserId(String userId) {
    this.userId = userId;
  }

  public String getName() {
    return name;
  }

  void setName(String name) {
    this.name = name;
  }

  public DeliveryStatus getStatus() {
    return status;
  }

  void setStatus(DeliveryStatus status) {
    this.status = status;
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

  public Date getActivityPeriodStart() {
    return activityPeriodStart;
  }

  void setActivityPeriodStart(Date activityPeriodStart) {
    this.activityPeriodStart = activityPeriodStart;
  }

  public Date getActivityPeriodEnd() {
    return activityPeriodEnd;
  }

  void setActivityPeriodEnd(Date activityPeriodEnd) {
    this.activityPeriodEnd = activityPeriodEnd;
  }
}
