package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryState;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryStatus;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Functions;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Конвертор сущностей рассылки
 *
 * @author Aleksandr Khalitov
 */
class DcpConverter {

  private static final TimeZone DCP_TIMEZONE=TimeZone.getTimeZone("UTC");

  private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

  private static final String TIME_FORMAT = "HH:mm:ss";

  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private static final String DATE_FORMAT_YY = "dd.MM.yy HH:mm:ss";

  private static final Logger logger = Logger.getLogger(DcpConverter.class);

  public static Time convertTimeFromDcpFormat(String time) throws AdminException {
    return new Time(time);
  }

//  public static String convertTimeToDcpFormat(Date time) {
//    SimpleDateFormat format = new SimpleDateFormat(TIME_FORMAT);
//    return format.format(time);
//  }

  public static String convertTimeToDcpFormat(Time time) { //todo
    String hoursStr = String.valueOf(time.getHour());
    if (hoursStr.length() == 1)
      hoursStr = '0' + hoursStr;
    String minStr = String.valueOf(time.getMin());
    if (minStr.length() == 1)
      minStr = '0' + minStr;
    String secStr = String.valueOf(time.getSec());
    if (secStr.length() == 1)
      secStr = '0' + secStr;

    return hoursStr + ':' + minStr + ':' + secStr;
  }

  public static Date convertDateFromDcpFormat(String date) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    try {
      return Functions.convertTime(format.parse(date), DCP_TIMEZONE, LOCAL_TIMEZONE);
    } catch (ParseException e) {
      throw new DeliveryException("unparsable_date", date);
    }
  }

  public static Date convertDateYYFromDcpFormat(String date) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT_YY);
    try {
      return Functions.convertTime(format.parse(date), DCP_TIMEZONE, LOCAL_TIMEZONE);
    } catch (ParseException e) {
      throw new DeliveryException("unparsable_date", date);
    }
  }

  public static String convertDateToDcpFormat(Date date) {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    return format.format(Functions.convertTime(date, LOCAL_TIMEZONE, DCP_TIMEZONE));
  }

  static Day[] convertDays(String[] days) {
    if (days == null) {
      return null;
    }
    Day[] res = new Day[days.length];
    for (int i = 0; i < days.length; i++) {
      res[i] = Day.valueOf(days[i]);
    }
    return res;
  }

  static String[] convertDays(Day[] days) {
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
    delivery.setActivePeriodEnd(new Time(convertTimeFromDcpFormat(di.getActivePeriodEnd())));
    delivery.setActivePeriodStart(new Time(convertTimeFromDcpFormat(di.getActivePeriodStart())));
    delivery.setActiveWeekDays(convertDays(di.getActiveWeekDays()));
    delivery.setDeliveryMode(convert(di.getDeliveryMode()));
    if (di.hasEndDate()) {
      delivery.setEndDate(convertDateFromDcpFormat(di.getEndDate()));
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
    delivery.setStartDate(convertDateFromDcpFormat(di.getStartDate()));
    if (di.hasSvcType()) {
      delivery.setSvcType(di.getSvcType());
    }
    if(di.hasArchivationPeriod()) {
      delivery.setArchiveTime(Integer.parseInt(di.getArchivationPeriod().substring(0, di.getArchivationPeriod().indexOf(":"))));
    }
    if(logger.isDebugEnabled()) {
      logger.debug("protogen archivationPeriod="+(di.hasArchivationPeriod() ? di.getArchivationPeriod() : "null"));
    }
    delivery.setTransactionMode(di.getTransactionMode());
    delivery.setUseDataSm(di.getUseDataSm());

    if (di.hasValidityPeriod()) {
      delivery.setValidityPeriod(new Time(convertTimeFromDcpFormat(di.getValidityPeriod())));
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
    if (ds.getDate() != null)
      result.setDate(convertDateToDcpFormat(ds.getDate()));
    result.setStatus(convert(ds.getStatus()));
    return result;
  }

  public static mobi.eyeline.informer.admin.delivery.DeliveryState convert(DeliveryState ds) throws AdminException {
    if (ds == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.DeliveryState result = new mobi.eyeline.informer.admin.delivery.DeliveryState();
    if (ds.hasDate())
      result.setDate(convertDateFromDcpFormat(ds.getDate()));
    result.setStatus(convert(ds.getStatus()));
    return result;
  }

  public static Message convert(mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo mi) throws AdminException {
    if (mi == null) {
      return null;
    }
    Message result = new Message();
    result.setId(mi.getId());
    if (mi.hasAbonent()) {
      result.setAbonent(new Address(mi.getAbonent()));
    }
    if (mi.hasDate()) {
      result.setDate(convertDateYYFromDcpFormat(mi.getDate()));
    }
    if (mi.hasErrorCode()) {
      result.setErrorCode(mi.getErrorCode());
    }
    if (mi.hasState()) {
      result.setState(convert(mi.getState()));
    }
    if (mi.hasText())
      result.setText(mi.getText());
    
    if (mi.hasUserData()) {
      Map<String, String> props = convertUserData(mi.getUserData());
      if (props != null)
        result.addProperties(props);
    }
    return result;
  }

  public static DeliveryMessage convert(Message m) {
    DeliveryMessage result = new DeliveryMessage();
    result.setAbonent(m.getAbonent().getSimpleAddress());
    if (m.getText() != null) {
      result.setText(m.getText());
      result.setMsgType(MessageType.TextMessage);
    } else {
      result.setIndex(m.getGlossaryIndex());
      result.setMsgType(MessageType.GlossaryMessage);
    }

    String userDataStr = convertUserData(m.getProperties());
    if (userDataStr != null)
      result.setUserData(userDataStr);
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

  public static Delivery convert(DeliveryListInfo di) throws AdminException {
    if (di == null) {
      return null;
    }
    Delivery result = new Delivery();
    result.setId(di.getDeliveryId());
    if (di.hasActivityPeriodEnd()) {
      result.setActivePeriodEnd(new Time(convertTimeFromDcpFormat(di.getActivityPeriodEnd())));
    }
    if (di.hasActivityPeriodStart()) {
      result.setActivePeriodStart(new Time(convertTimeFromDcpFormat(di.getActivityPeriodStart())));
    }
    if (di.hasEndDate()) {
      result.setEndDate(convertDateYYFromDcpFormat(di.getEndDate()));
    }
    if (di.hasName()) {
      result.setName(di.getName());
    }
    if (di.hasStartDate()) {
      result.setStartDate(convertDateYYFromDcpFormat(di.getStartDate()));
    }
    if (di.hasStatus()) {
      result.setStatus(convert(di.getStatus()));
    }
    if (di.hasUserId()) {
      result.setOwner(di.getUserId());
    }
    if (di.hasUserData()) {
      Map<String, String> uD = convertUserData(di.getUserData());
      if (uD != null)
        result.addProperties(uD);
    }
    return result;
  }

  public static mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo convert(Delivery di) {
    if (di == null) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo delivery = new mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo();
    delivery.setActivePeriodEnd(convertTimeToDcpFormat(di.getActivePeriodEnd()));
    delivery.setActivePeriodStart(convertTimeToDcpFormat(di.getActivePeriodStart()));
    delivery.setActiveWeekDays(convertDays(di.getActiveWeekDays()));
    delivery.setDeliveryMode(convert(di.getDeliveryMode()));
    if (di.getEndDate() != null) {
      delivery.setEndDate(convertDateToDcpFormat(di.getEndDate()));
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
    delivery.setStartDate(convertDateToDcpFormat(di.getStartDate()));
    if (di.getSvcType() != null) {
      delivery.setSvcType(di.getSvcType());
    }
    if(di.getArchiveTime() != null) {
      StringBuilder sb = new StringBuilder(8);
      if(di.getArchiveTime()<10) {
        sb.append('0');
      }
      sb.append(di.getArchiveTime()).append(":00:00");
      delivery.setArchivationPeriod(sb.toString());
    }
    delivery.setTransactionMode(di.isTransactionMode());
    delivery.setUseDataSm(di.isUseDataSm());

    if (di.getValidityPeriod() != null)
      delivery.setValidityPeriod(convertTimeToDcpFormat(di.getValidityPeriod()));

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
    result.setSentMessages(stats.getSentMessages());
    result.setRetriedMessages(stats.getRetriedMessages());
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
      items.add(new DeliveryStatusHistory.Item(convertDateYYFromDcpFormat(i.getDate()), convert(i.getStatus())));
    }
    return new DeliveryStatusHistory(deliveryId, items);
  }
}
