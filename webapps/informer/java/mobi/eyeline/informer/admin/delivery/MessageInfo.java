package mobi.eyeline.informer.admin.delivery;

import java.text.SimpleDateFormat;
import java.util.Date;

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
  private Integer index;
  private Integer errorCode;
  private String userData;

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

  public Integer getIndex() {
    return index;
  }

  void setIndex(Integer index) {
    this.index = index;
  }

  public Integer getErrorCode() {
    return errorCode;
  }

  void setErrorCode(Integer errorCode) {
    this.errorCode = errorCode;
  }

  public String getUserData() {
    return userData;
  }

  void setUserData(String userData) {
    this.userData = userData;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    MessageInfo that = (MessageInfo) o;

    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    if (id != that.id) return false;
    if (abonent != null ? !abonent.equals(that.abonent) : that.abonent != null) return false;
    if (date != null ? !dateFormat.format(date).equals(that.date == null ? null : dateFormat.format(that.date)) : that.date != null) return false;
    if (errorCode != null ? !errorCode.equals(that.errorCode) : that.errorCode != null) return false;
    if (index != null ? !index.equals(that.index) : that.index != null) return false;
    if (state != that.state) return false;
    if (text != null ? !text.equals(that.text) : that.text != null) return false;
    if (userData != null ? !userData.equals(that.userData) : that.userData != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }
}
