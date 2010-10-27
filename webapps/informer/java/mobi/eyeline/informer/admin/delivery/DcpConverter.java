package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

/**
 * Конвертор сущностей рассылки
 *
 * @author Aleksandr Khalitov
 */
public class DcpConverter {

  private static final String TIME_FORMAT = "HH:mm:ss";

  private static final String DATE_FORMAT = "dd:MM:yyyy HH:mm:ss";

  public static Date convertTime(String time) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(TIME_FORMAT);
    try {
      return format.parse(time);
    } catch (ParseException e) {
      throw new DcpConverterException("unparsable_date", time);
    }
  }

  public static String convertTime(Date time) {
    SimpleDateFormat format = new SimpleDateFormat(TIME_FORMAT);
    return format.format(time);
  }

  public static Date convertDate(String date) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    try {
      return format.parse(date);
    } catch (ParseException e) {
      throw new DcpConverterException("unparsable_date", date);
    }
  }

  public static String convertDate(Date date) {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    return format.format(date);
  }

  static Delivery.Day[] convertDays(String[] days) {
    if (days == null) {
      return null;
    }
    Delivery.Day[] res = new Delivery.Day[days.length];
    for (int i = 0; i < days.length; i++) {
      res[i] = Delivery.Day.valueOf(days[i]);
    }
    return res;
  }

  static String[] convertDays(Delivery.Day[] days) {
    if (days == null) {
      return null;
    }
    String[] res = new String[days.length];
    for (int i = 0; i < days.length; i++) {
      res[i] = days[i].toString();
    }
    return res;
  }

  static DeliveryMode convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMode mode) {
    if (mode == null) {
      return null;
    }
    return DeliveryMode.valueOf(mode.toString());
  }

  static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMode convert(DeliveryMode mode) {
    if (mode == null) {
      return null;
    }
    return mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryMode.valueOf(mode.toString());
  }

  public static Delivery convert(int id, mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo di) throws AdminException {
    if (di == null) {
      return null;
    }
    Delivery delivery = new Delivery();
    delivery.setId(id);
    delivery.setActivePeriodEnd(convertTime(di.getActivePeriodEnd()));
    delivery.setActivePeriodStart(convertTime(di.getActivePeriodStart()));
    delivery.setActiveWeekDays(convertDays(di.getActiveWeekDays()));
    delivery.setDeliveryMode(convert(di.getDeliveryMode()));
    delivery.setEndDate(convertDate(di.getEndDate()));
    delivery.setFlash(di.getFlash());
    delivery.setName(di.getName());
    delivery.setOwner(di.getOwner());
    delivery.setPriority(di.getPriority());
    delivery.setReplaceMessage(di.getReplaceMessage());
    delivery.setRetryOnFail(di.getRetryOnFail());
    delivery.setRetryPolicy(di.getRetryPolicy());
    String userData = di.getUserData();
    if(userData != null) {
      Map<String, String> uD = convertUserData(userData);
      String t = uD.get("secret");
      delivery.setSecret(t != null && Boolean.valueOf(t));
      t = uD.get("secretFlash");
      delivery.setSecretFlash(t != null && Boolean.valueOf(t));
      t = uD.get("secretMessage");
      if(t != null) {
        delivery.setSecretMessage(t);
      }
    }
    delivery.setStartDate(convertDate(di.getStartDate()));
    if (di.hasSvcType()) {
      delivery.setSvcType(di.getSvcType());
    }
    delivery.setTransactionMode(di.getTransactionMode());
    delivery.setUseDataSm(di.getUseDataSm());

    if (di.hasValidityDate()) {
      delivery.setValidityDate(convertDate(di.getValidityDate()));
    }
    if (di.hasValidityPeriod()) {
      delivery.setValidityPeriod(di.getValidityPeriod());
    }
    return delivery;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState convert(mobi.eyeline.informer.admin.delivery.DeliveryState ds) {
    if (ds == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState result = new DeliveryState();
    result.setDate(convertDate(ds.getDate()));
    result.setStatus(convert(ds.getStatus()));
    return result;
  }

  public static mobi.eyeline.informer.admin.delivery.DeliveryState convert(DeliveryState ds) throws AdminException {
    if (ds == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.DeliveryState result = new mobi.eyeline.informer.admin.delivery.DeliveryState();
    result.setDate(convertDate(ds.getDate()));
    result.setStatus(convert(ds.getStatus()));
    return result;
  }

  public static MessageInfo convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo mi) throws AdminException {
    if (mi == null) {
      return null;
    }
    MessageInfo result = new MessageInfo();
    result.setId(mi.getId());
    if (mi.hasAbonent()) {
      result.setAbonent(mi.getAbonent());
    }
    if (mi.hasDate()) {
      result.setDate(convertDate(mi.getDate()));
    }
    if (mi.hasErrorCode()) {
      result.setErrorCode(mi.getErrorCode());
    }
    if (mi.hasIndex()) {
      result.setIndex(mi.getIndex());
    }
    if (mi.hasState()) {
      result.setState(convert(mi.getState()));
    }
    if (mi.hasText()) {
      result.setText(mi.getText());
    }
    if (mi.hasUserData()) {
      result.setUserData(mi.getUserData());
    }
    return result;
  }

  public static DeliveryMessage convert(Message m) {
    DeliveryMessage result = new DeliveryMessage();
    result.setAbonent(m.getAbonent().getSimpleAddress());
    if (m.getIndex() != null) {
      result.setIndex(m.getIndex());
    } else {
      result.setText(m.getText());
    }
    return result;
  }
//
//  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageType convert(MessageType mType) {
//    if (mType == null) {
//      return null;
//    }
//    return mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageType.valueOf(mType.toString());
//  }
//
//  public static MessageType convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageType mType) {
//    if (mType == null) {
//      return null;
////    }
//    return MessageType.valueOf(mType.toString());
//  }

  public static DeliveryMessage[] convert(Message[] dm) {
    if (dm == null) {
      return null;
    }
    DeliveryMessage[] result = new DeliveryMessage[dm.length];
    for (int i = 0; i < dm.length; i++) {
      result[i] = convert(dm[i]);
    }
    return result;
  }

  public static DeliveryInfo convert(DeliveryListInfo di) throws AdminException {
    if (di == null) {
      return null;
    }
    DeliveryInfo result = new DeliveryInfo();
    result.setDeliveryId(di.getDeliveryId());
    if (di.hasActivityPeriodEnd()) {
      result.setActivityPeriodEnd(convertTime(di.getActivityPeriodEnd()));
    }
    if (di.hasActivityPeriodStart()) {
      result.setActivityPeriodStart(convertTime(di.getActivityPeriodStart()));
    }
    if (di.hasEndDate()) {
      result.setEndDate(convertDate(di.getEndDate()));
    }
    if (di.hasName()) {
      result.setName(di.getName());
    }
    if (di.hasStartDate()) {
      result.setStartDate(convertDate(di.getStartDate()));
    }
    if (di.hasStatus()) {
      result.setStatus(convert(di.getStatus()));
    }
    if (di.hasUserId()) {
      result.setUserId(di.getUserId());
    }
    return result;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo convert(Delivery di) {
    if (di == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo delivery = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo();
    delivery.setActivePeriodEnd(convertTime(di.getActivePeriodEnd()));
    delivery.setActivePeriodStart(convertTime(di.getActivePeriodStart()));
    delivery.setActiveWeekDays(convertDays(di.getActiveWeekDays()));
    delivery.setDeliveryMode(convert(di.getDeliveryMode()));
    delivery.setEndDate(convertDate(di.getEndDate()));
    delivery.setFlash(di.isFlash());
    delivery.setName(di.getName());
    delivery.setOwner(di.getOwner());
    delivery.setPriority(di.getPriority());
    delivery.setReplaceMessage(di.isReplaceMessage());
    delivery.setRetryOnFail(di.isRetryOnFail());
    delivery.setRetryPolicy(di.getRetryPolicy());
    Map<String, Object> userData = new HashMap<String, Object>(3);
    userData.put("secret", di.isSecret());
    userData.put("secretFlash", di.isSecretFlash());
    if(di.getSecretMessage() != null) {
      userData.put("secretMessage", di.getSecretMessage());
    }
    delivery.setUserData(convertUserData(userData));
    delivery.setStartDate(convertDate(di.getStartDate()));
    if (di.getSvcType() != null) {
      delivery.setSvcType(di.getSvcType());
    }
    delivery.setTransactionMode(di.isTransactionMode());
    delivery.setUseDataSm(di.isUseDataSm());

    if (di.getValidityDate() != null) {
      delivery.setValidityDate(convertDate(di.getValidityDate()));
    }
    if (di.getValidityPeriod() != null) {
      delivery.setValidityPeriod(di.getValidityPeriod());
    }
    return delivery;
  }

  public static DeliveryStatus[] convert(mobi.eyeline.informer.admin.delivery.DeliveryStatus[] statuses) {
    if (statuses == null) {
      return null;
    }
    DeliveryStatus[] result = new DeliveryStatus[statuses.length];
    for (int i = 0; i < statuses.length; i++) {
      result[i] = convert(statuses[i]);
    }
    return result;
  }

  public static DeliveryStatus convert(mobi.eyeline.informer.admin.delivery.DeliveryStatus status) {
    if (status == null) {
      return null;
    }
    return DeliveryStatus.valueOf(status.toString());
  }


  public static mobi.eyeline.informer.admin.delivery.DeliveryStatus[] convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus[] statuses) {
    if (statuses == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.DeliveryStatus[] result = new mobi.eyeline.informer.admin.delivery.DeliveryStatus[statuses.length];
    for (int i = 0; i < statuses.length; i++) {
      result[i] = convert(statuses[i]);
    }
    return result;
  }

  public static mobi.eyeline.informer.admin.delivery.DeliveryStatus convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus status) {
    if (status == null) {
      return null;
    }
    return mobi.eyeline.informer.admin.delivery.DeliveryStatus.valueOf(status.toString());
  }


  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[] convert(mobi.eyeline.informer.admin.delivery.DeliveryFields[] fields) {
    if (fields == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[] result = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields[fields.length];
    for (int i = 0; i < fields.length; i++) {
      result[i] = convert(fields[i]);
    }
    return result;
  }

  private static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields convert(mobi.eyeline.informer.admin.delivery.DeliveryFields fields) {
    if (fields == null) {
      return null;
    }
    return mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields.valueOf(fields.toString());
  }

  public static ReqField[] convert(MessageFields[] fields) {
    if (fields == null) {
      return null;
    }
    ReqField[] result = new ReqField[fields.length];
    for (int i = 0; i < fields.length; i++) {
      result[i] = convert(fields[i]);
    }
    return result;
  }

  private static ReqField convert(MessageFields fields) {
    if (fields == null) {
      return null;
    }
    return ReqField.valueOf(fields.toString());
  }


  public static DeliveryMessageState[] convert(MessageState[] states) {
    if (states == null) {
      return null;
    }
    DeliveryMessageState[] result = new DeliveryMessageState[states.length];
    for (int i = 0; i < states.length; i++) {
      result[i] = convert(states[i]);
    }
    return result;
  }

  public static DeliveryMessageState convert(MessageState state) {
    if (state == null) {
      return null;
    }
    return DeliveryMessageState.valueOf(state.toString());
  }

  public static MessageState convert(DeliveryMessageState state) {
    if (state == null) {
      return null;
    }
    return MessageState.valueOf(state.toString());
  }

  public static DeliveryStatistics convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatistics stats,
                                           mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState state) throws AdminException {
    if (stats == null || state == null) {
      return null;
    }
    DeliveryStatistics result = new DeliveryStatistics();
    result.setDeliveryState(convert(state));
    result.setDeliveredMessages(stats.getDeliveredMessages());
    result.setExpiredMessages(stats.getExpiredMessages());
    result.setNewMessages(stats.getNewMessages());
    result.setFailedMessage(stats.getFailedMessage());
    result.setProcessMessage(stats.getProcessMessage());
    return result;
  }


  private static String convertUserData(Map<String, Object> m) {
    if(m == null || m.isEmpty()) {
      return null;
    }
    StringBuilder sb = new StringBuilder();
    for(Map.Entry<String, Object> e : m.entrySet()) {
      sb.append(';').append(e.getKey()).append('=').append(e.getValue());
    }
    return sb.substring(1);
  }

  private static Map<String, String> convertUserData(String s) {
    if(s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    String[] entries = s.split(";");
    Map<String, String> r = new HashMap<String, String>(entries.length);
    for(String e : entries) {
      String[] kv = e.split("=");
      r.put(kv[0], kv[1]);
    }
    return r;
  }
}
