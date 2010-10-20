package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

/**
 * @author Aleksandr Khalitov
 */
public class Message {

  private static final ValidationHelper vh = new ValidationHelper(Message.class);

  private String id;

  private MessageType msgType;
  private String text;
  private int index;

  public static Message newGlossaryMessage(int index) {
    return null;
  }

  public static Message newTextMessage(String text) {
    return null;
  }

  private Message(MessageType mType, int index,String text) {

  }

  public String getId() {
    return id;
  }

  void setId(String id) {
    this.id = id;
  }

  public MessageType getMsgType() {
    return msgType;
  }

  public void setMsgType(MessageType msgType) throws AdminException{
    vh.checkNotNull("msgType", msgType);
    this.msgType = msgType;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  public int getIndex() {
    return index;
  }

  public void setIndex(int index) {
    this.index = index;
  }
}
