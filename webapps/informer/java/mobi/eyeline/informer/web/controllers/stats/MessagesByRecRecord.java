package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 26.10.2010
 * Time: 15:47:35
 */
public class MessagesByRecRecord {

  private final String recipient;

  private final String userId;
  private final User user;
  private final Integer deliveryId;

  private final String name;
  private final String text;
  private final Date deliveryDate;
  private final MessageState state;
  private final String errorString;

  public MessagesByRecRecord(String recipient, Integer deliveryId, String userId, User user, String name, String text, Date deliveryDate, MessageState state, String errorString) {
    this.recipient = recipient;
    this.deliveryId = deliveryId;
    this.userId = userId;
    this.user = user;
    this.name = name;
    this.text = text;
    this.deliveryDate = deliveryDate;
    this.state = state;
    this.errorString = errorString;
  }

  public String getRecipient() {
    return recipient;
  }

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public String getUserId() {
    return userId;
  }

  public String getName() {
    return name;
  }

  public String getText() {
    return text;
  }

  public Date getDeliveryDate() {
    return deliveryDate;
  }

  public String getDeliveryDateString() {
    if (deliveryDate == null) return null;
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(deliveryDate);
  }

  public MessageState getState() {
    return state;
  }

  public String getErrorString() {
    return errorString;
  }

  public String getUserDetails() {
    return user == null ? "" : user.getFirstName() + " " + user.getLastName();
  }

  public void printCSV(PrintWriter writer) {
    writer.println(StringEncoderDecoder.toCSVString(recipient,name,
        userId,
        text,
        getDeliveryDateString(),
        state.toString(),
        errorString));
  }


}
