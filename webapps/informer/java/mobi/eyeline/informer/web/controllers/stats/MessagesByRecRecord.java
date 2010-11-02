package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 26.10.2010
 * Time: 15:47:35
 */
public class MessagesByRecRecord {

  private  String  userId;
  private  User user;
  private  Integer deliveryId;

  private  String  name;
  private  String  text;
  private  Date    deliveryDate;
  private  MessageState state;
  private  String errorString;

  public MessagesByRecRecord(Integer deliveryId, String userId, User user, String name, String text, Date deliveryDate, MessageState state, String errorString) {
    this.deliveryId = deliveryId;
    this.userId = userId;
    this.user = user;
    this.name = name;
    this.text = text;
    this.deliveryDate = deliveryDate;
    this.state = state;
    this.errorString = errorString;
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
    if(deliveryDate==null) return null;
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(deliveryDate);
  }

  public MessageState getState() {
    return state;
  }

  public String getErrorString() {
    return errorString;
  }

  public String getUserDetails() {
    return user==null ? "" : user.getFirstName()+" "+user.getLastName();
  }

  public void printCSV(PrintWriter writer) {
    writer.println(StringEncoderDecoder.toCSVString(new Object[]{
      name,
      userId,
      text,
      getDeliveryDateString(),
      state.toString(),
      errorString
    }));
  }


}
