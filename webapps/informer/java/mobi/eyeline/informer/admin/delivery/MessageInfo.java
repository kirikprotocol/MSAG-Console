package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Информация о состоянии рассылки
 * @author Aleksandr Khalitov
 */
public class MessageInfo {

  private int id;
  private MessageState state;
  private Date date;
  private String abonent;
  private String text;
  private Integer index;
  private Integer errorCode;
  private String userData;

  public int getId() {
    return id;
  }

  void setId(int id) {
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
}
