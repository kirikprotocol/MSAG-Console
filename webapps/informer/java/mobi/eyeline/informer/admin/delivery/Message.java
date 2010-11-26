package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * Сообщение рассылки
 *
 * @author Aleksandr Khalitov
 */
public class Message {

  private static final ValidationHelper vh = new ValidationHelper(Message.class);

  private Long id;
  private MessageState state;
  private Date date;
  private String text;
  private int glossaryIndex;
  private Address abonent;
  private Integer errorCode;

  private final Properties properties = new Properties();

  public static Message newMessage(String text) {
    return new Message(text);
  }

  public static Message newMessage(Address abonent, String text) {
    return new Message(abonent, text);
  }

  private Message(String text) {
    this.text = text;
  }

  private Message(Address abonent, String text) {
    this.abonent = abonent;
    this.text = text;
  }

  protected Message() {
  }

  void setText(String text) {
    this.text = text;
  }

  public Long getId() {
    return id;
  }

  void setId(Long id) {
    this.id = id;
  }

  public Address getAbonent() {
    return abonent;
  }

  public void setAbonent(Address abonent) throws AdminException {
    vh.checkNotNull("msisdn", abonent);
    this.abonent = abonent;
  }

  public MessageState getState() {
    return state;
  }

  public void setState(MessageState state) {
    this.state = state;
  }

  public Date getDate() {
    return date;
  }

  public void setDate(Date date) {
    this.date = date;
  }

  public Integer getErrorCode() {
    return errorCode;
  }

  public void setErrorCode(Integer errorCode) {
    this.errorCode = errorCode;
  }

  public String removeProperty(String name) {
    return (String) properties.remove(name);
  }

  public void setProperty(String name, String value) {
    properties.setProperty(name, value);
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

  int getGlossaryIndex() {
    return glossaryIndex;
  }

  void setGlossaryIndex(int glossaryIndex) {
    this.glossaryIndex = glossaryIndex;
  }

  public String getText() {
    return text;
  }

  public Message cloneMessage() {
    Message m = new Message(text);
    m.id = id;
    m.abonent = abonent;
    m.glossaryIndex = glossaryIndex;
    m.state = state;
    m.date = date;
    m.errorCode = errorCode;
    return m;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Message message = (Message) o;

    if (id != null ? !id.equals(message.id) : message.id != null) return false;
    if (abonent != null ? !abonent.equals(message.abonent) : message.abonent != null) return false;
    if (text != null ? !text.equals(message.text) : message.text != null) return false;
    if ((properties != null && message.properties == null) || (properties == null && message.properties != null)) {
      return false;
    }
    if (properties != null) {
      if (properties.size() != message.properties.size()) {
        return false;
      }
      for (Map.Entry e : properties.entrySet()) {
        Object v;
        if (((v = message.properties.get(e.getKey())) == null) || !v.equals(e.getValue())) {
          return false;
        }
      }
    }

    return true;
  }

  @Override
  public int hashCode() {
    int result = id != null ? id.hashCode() : 0;
    result = 31 * result + (text != null ? text.hashCode() : 0);
    result = 31 * result + (abonent != null ? abonent.hashCode() : 0);
    return result;
  }
}
