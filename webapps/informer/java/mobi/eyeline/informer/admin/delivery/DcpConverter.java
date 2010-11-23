package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Конвертор сущностей рассылки
 *
 * @author Aleksandr Khalitov
 */
public class DcpConverter {

  private static final String TIME_FORMAT = "HH:mm:ss";

  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private static final String DATE_FORMAT_YY = "dd.MM.yy HH:mm:ss";

  public static Date convertTime(String time) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(TIME_FORMAT);
    try {
      return format.parse(time);
    } catch (ParseException e) {
      throw new DeliveryException("unparsable_date", time);
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
      throw new DeliveryException("unparsable_date", date);
    }
  }

  public static Date convertDateYY(String date) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT_YY);
    try {
      return format.parse(date);
    } catch (ParseException e) {
      throw new DeliveryException("unparsable_date", date);
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

  public static Delivery convert(int id, mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo di, String[] glossary) throws AdminException {
    if (di == null) {
      return null;
    }
    Delivery delivery;
    if (glossary == null) {
      delivery = Delivery.newCommonDelivery();
    } else {
      delivery = Delivery.newSingleTextDelivery();
      delivery.setSingleText(glossary[0]);
    }
    delivery.setId(id);
    delivery.setActivePeriodEnd(new Time(convertTime(di.getActivePeriodEnd())));
    delivery.setActivePeriodStart(new Time(convertTime(di.getActivePeriodStart())));
    delivery.setActiveWeekDays(convertDays(di.getActiveWeekDays()));
    delivery.setDeliveryMode(convert(di.getDeliveryMode()));
    if (di.hasEndDate()) {
      delivery.setEndDate(convertDate(di.getEndDate()));
    }
    delivery.setFlash(di.getFlash());
    delivery.setName(di.getName());
    delivery.setOwner(di.getOwner());
    delivery.setPriority(di.getPriority());
    delivery.setReplaceMessage(di.getReplaceMessage());
    delivery.setRetryOnFail(di.getRetryOnFail());
    if (di.hasRetryPolicy())
      delivery.setRetryPolicy(di.getRetryPolicy());
    delivery.setSourceAddress(new Address(di.getSourceAddress()));

    if (di.hasUserData()) {
      Map<String, String> uD = convertUserData(di.getUserData());
      delivery.addProperties(uD);
    }
    delivery.setStartDate(convertDate(di.getStartDate()));
    if (di.hasSvcType()) {
      delivery.setSvcType(di.getSvcType());
    }
    delivery.setTransactionMode(di.getTransactionMode());
    delivery.setUseDataSm(di.getUseDataSm());

    if (di.hasValidityPeriod()) {
      delivery.setValidityPeriod(new Time(convertTime(di.getValidityPeriod())));
    }

    delivery.setEnableMsgFinalizationLogging(di.getFinalMsgRecords());
    delivery.setEnableStateChangeLogging(di.getFinalDlvRecords());
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
    if (ds.hasDate())
      result.setDate(convertDate(ds.getDate()));
    result.setStatus(convert(ds.getStatus()));
    return result;
  }

  public static MessageInfo convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo mi, String text) throws AdminException {
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
    if (mi.hasState()) {
      result.setState(convert(mi.getState()));
    }
    if (text == null && mi.hasText()) {
      result.setText(mi.getText());
    } else {
      result.setText(text);
    }
    if (mi.hasUserData()) {
      result.addProperties(convertUserData(mi.getUserData()));
    }
    return result;
  }

  public static DeliveryMessage convert(Message m) {
    DeliveryMessage result = new DeliveryMessage();
    result.setAbonent(m.getAbonent().getSimpleAddress());
    result.setText(m.getText());
    result.setMsgType(MessageType.TextMessage);
    result.setUserData(convertUserData(m.getProperties()));
    return result;
  }

  public static DeliveryMessage[] convert(Collection<Message> dm) {
    if (dm == null) {
      return null;
    }
    DeliveryMessage[] result = new DeliveryMessage[dm.size()];
    int i = 0;
    for (Message m : dm) {
      result[i] = convert(m);
      i++;
    }
    return result;
  }

  public static DeliveryMessage[] convert(List<Address> addresses) {
    if (addresses == null) {
      return null;
    }
    DeliveryMessage[] result = new DeliveryMessage[addresses.size()];
    for (int i = 0; i < addresses.size(); i++) {
      result[i] = new DeliveryMessage();
      result[i].setIndex(0);
      result[i].setAbonent(addresses.get(i).getSimpleAddress());
      result[i].setMsgType(MessageType.GlossaryMessage);
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
      result.setActivityPeriodEnd(new Time(convertTime(di.getActivityPeriodEnd())));
    }
    if (di.hasActivityPeriodStart()) {
      result.setActivityPeriodStart(new Time(convertTime(di.getActivityPeriodStart())));
    }
    if (di.hasEndDate()) {
      result.setEndDate(convertDateYY(di.getEndDate()));
    }
    if (di.hasName()) {
      result.setName(di.getName());
    }
    if (di.hasStartDate()) {
      result.setStartDate(convertDateYY(di.getStartDate()));
    }
    if (di.hasStatus()) {
      result.setStatus(convert(di.getStatus()));
    }
    if (di.hasUserId()) {
      result.setUserId(di.getUserId());
    }
    if (di.hasUserData()) {
      Map<String, String> uD = convertUserData(di.getUserData());
      result.addProperties(uD);
    }
    return result;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo convert(Delivery di) {
    if (di == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo delivery = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo();
    delivery.setActivePeriodEnd(convertTime(di.getActivePeriodEnd().getTimeDate()));
    delivery.setActivePeriodStart(convertTime(di.getActivePeriodStart().getTimeDate()));
    delivery.setActiveWeekDays(convertDays(di.getActiveWeekDays()));
    delivery.setDeliveryMode(convert(di.getDeliveryMode()));
    if (di.getEndDate() != null) {
      delivery.setEndDate(convertDate(di.getEndDate()));
    }
    delivery.setFlash(di.isFlash());
    delivery.setName(di.getName());
    delivery.setOwner(di.getOwner());
    delivery.setPriority(di.getPriority());
    delivery.setReplaceMessage(di.isReplaceMessage());
    delivery.setRetryOnFail(di.isRetryOnFail());
    if (di.getRetryPolicy() != null)
      delivery.setRetryPolicy(di.getRetryPolicy());
    delivery.setSourceAddress(di.getSourceAddress().getSimpleAddress());
    if (di.getProperties() != null)
      delivery.setUserData(convertUserData(di.getProperties()));
    delivery.setStartDate(convertDate(di.getStartDate()));
    if (di.getSvcType() != null) {
      delivery.setSvcType(di.getSvcType());
    }
    delivery.setTransactionMode(di.isTransactionMode());
    delivery.setUseDataSm(di.isUseDataSm());

    if (di.getValidityPeriod() != null)
      delivery.setValidityPeriod(convertTime(di.getValidityPeriod().getTimeDate()));

    delivery.setFinalDlvRecords(di.isEnableStateChangeLogging());
    delivery.setFinalMsgRecords(di.isEnableMsgFinalizationLogging());

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
    result.setFailedMessages(stats.getFailedMessage());
    result.setProcessMessages(stats.getProcessMessage());
    return result;
  }


  public static String convertUserData(Map m) {
    if (m == null || m.isEmpty()) {
      return null;
    }
    StringBuilder sb = new StringBuilder();
    for (Object e1 : m.entrySet()) {
      Map.Entry e = (Map.Entry)e1;      
      sb.append(';').append(e.getKey()).append('=').append(e.getValue());
    }
    return sb.substring(1);
  }

  public static Map<String, String> convertUserData(String s) {
    if (s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    String[] entries = s.split(";");
    Map<String, String> r = new HashMap<String, String>(entries.length);
    for (String e : entries) {
      String[] kv = e.split("=");
      r.put(kv[0], kv[1]);
    }
    return r;
  }

  public static DeliveryStatusHistory convert(int deliveryId, DeliveryHistoryItem[] history) throws AdminException {
    if (history == null) {
      return null;
    }
    List<DeliveryStatusHistory.Item> items = new ArrayList<DeliveryStatusHistory.Item>(history.length);
    for (DeliveryHistoryItem i : history) {
      items.add(new DeliveryStatusHistory.Item(convertDate(i.getDate()), convert(i.getStatus())));
    }
    return new DeliveryStatusHistory(deliveryId, items);
  }
}
