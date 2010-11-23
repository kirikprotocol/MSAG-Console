package mobi.eyeline.informer.admin.delivery;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * Информация о состоянии рассылки
 *
 * @author Aleksandr Khalitov
 */
public class MessageInfo {

  private long id;
  private MessageState state;
  private Date date;
  private String abonent;
  private String text;
  private Integer errorCode;

  private final Properties properties = new Properties();

  MessageInfo() {
  }

  public long getId() {
    return id;
  }

  void setId(long id) {
    this.id = id;
  }

  public MessageState getState() {
    return state;
  }

  void setState(MessageState state) {
    this.state = state;
  }

  public Date getDate() {
    return date;
  }

  void setDate(Date date) {
    this.date = date;
  }

  public String getAbonent() {
    return abonent;
  }

  void setAbonent(String abonent) {
    this.abonent = abonent;
  }

  public String getText() {
    return text;
  }

  void setText(String text) {
    this.text = text;
  }

  public Integer getErrorCode() {
    return errorCode;
  }

  void setErrorCode(Integer errorCode) {
    this.errorCode = errorCode;
  }

  public String removeProperty(String name) {
    return (String) properties.remove(name);
  }

  public void setProperty(String name, String value) {
    properties.setProperty(name, value);
  }

  public void addProperties(Map<String, String> props) {
    if (props != null)
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


  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    MessageInfo that = (MessageInfo) o;

    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    if (id != that.id) return false;
    if (abonent != null ? !abonent.equals(that.abonent) : that.abonent != null) return false;
    if (date != null ? !dateFormat.format(date).equals(that.date == null ? null : dateFormat.format(that.date)) : that.date != null)
      return false;
    if (errorCode != null ? !errorCode.equals(that.errorCode) : that.errorCode != null) return false;
    if (state != that.state) return false;
    if (text != null ? !text.equals(that.text) : that.text != null) return false;
    if ((properties != null && that.properties == null) || (properties == null && that.properties != null)) {
      return false;
    }
    if (properties != null) {
      if (properties.size() != that.properties.size()) {
        return false;
      }
      for (Map.Entry e : properties.entrySet()) {
        Object v;
        if (((v = that.properties.get(e.getKey())) == null) || !v.equals(e.getValue())) {
          return false;
        }
      }
    }

    return true;
  }
}
