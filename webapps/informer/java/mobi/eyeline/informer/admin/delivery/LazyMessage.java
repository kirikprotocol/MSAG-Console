package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 *
 * Сообщение, "лениво" десериализуемое из MessageInfo
 *
 * @author Artem Snopkov
 */
public class LazyMessage extends Message {

  private static final Logger log = Logger.getLogger(LazyMessage.class);

  private final MessageInfo info;
  private boolean propertiesLoaded;

  LazyMessage(MessageInfo info) {
    this.info = info;
  }

  public Long getId() {
    if (id == null)
      id = info.getId();
    return id;
  }

  public Address getAbonent() {
    if (abonent == null)
      abonent = new Address(info.getAbonent());
    return abonent;
  }

  public MessageState getState() {
    if (state == null)
      state = DcpConverter.convert(info.getState());
    return state;
  }

  public Date getDate() {
    if (date == null)
      try {
        date = DcpConverter.convertDateYYFromDcpFormat(info.getDate());
      } catch (AdminException e) {
        log.error(e,e);
      }
    return date;
  }


  public Integer getErrorCode() {
    if (errorCode == null)
      errorCode = info.getErrorCode();
    return errorCode;
  }

  private void loadProperties() {
    Map<String, String> props = DcpConverter.convertUserData(info.getUserData());
    if (props != null)
      properties.putAll(props);
    propertiesLoaded = true;
  }

  public String removeProperty(String name) {
    if (!propertiesLoaded)
      loadProperties();
    return (String) properties.remove(name);
  }

  public void setProperty(String name, String value) {
    if (!propertiesLoaded)
      loadProperties();
    properties.setProperty(name, value);
  }

  public void addProperties(Map<String, String> props) {
    if (!propertiesLoaded)
      loadProperties();
    properties.putAll(props);
  }

  public String getProperty(String name) {
    if (!propertiesLoaded)
      loadProperties();
    return properties.getProperty(name);
  }

  public boolean containsProperty(String name) {
    if (!propertiesLoaded)
      loadProperties();
    return properties.containsKey(name);
  }

  public Properties getProperties() {
    if (!propertiesLoaded)
      loadProperties();
    Properties properties = new Properties();
    properties.putAll(this.properties);
    return properties;
  }

  public String getText() {
    if (text == null)
      text = info.getText();
    return text;
  }

}
