package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

/**
 * Сообщение рассылки
 *
 * @author Aleksandr Khalitov
 */
public class Message {

  private static final ValidationHelper vh = new ValidationHelper(Message.class);

  private Long id;
  private String text;
  private Integer index;
  private Address abonent;


  public static Message newGlossaryMessage(int index) {
    return new Message(index, null);
  }

  public static Message newTextMessage(String text) {
    return new Message(null, text);
  }

  private Message(Integer index, String text) {
    this.index = index;
    this.text = text;
  }

  protected Message() {
  }

  void setText(String text) {
    this.text = text;
  }

  void setIndex(Integer index) {
    this.index = index;
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


  public String getText() {
    return text;
  }

  public Integer getIndex() {
    return index;
  }

  public Message cloneMessage() {
    Message m = new Message(index, text);
    m.id = id;
    m.abonent = abonent;
    return m;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Message message = (Message) o;

    if (id != null ? !id.equals(message.id) : message.id != null) return false;
    if (index != null ? !index.equals(message.index) : message.index != null) return false;
    if (abonent != null ? !abonent.equals(message.abonent) : message.abonent != null) return false;
    if (text != null ? !text.equals(message.text) : message.text != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }
}
