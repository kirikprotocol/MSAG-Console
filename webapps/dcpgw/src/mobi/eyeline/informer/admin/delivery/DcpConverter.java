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

  public static String convertTimeToDcpFormat(Time time) {
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
    return convertDateFromDcpFormat(date, true);
  }

  public static Date convertDateFromDcpFormat(String date, boolean convertTimezone) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    try {
      Date d = format.parse(date);
      return convertTimezone ? Functions.convertTime(d, DCP_TIMEZONE, LOCAL_TIMEZONE) : d;
    } catch (ParseException e) {
      throw new DeliveryException("unparsable_date", date);
    }
  }

  public static Date convertDateYYFromDcpFormat(String date) throws AdminException {
    return convertDateYYFromDcpFormat(date, true);
  }

  public static Date convertDateYYFromDcpFormat(String date, boolean convertTimezone) throws AdminException {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT_YY);
    try {
      Date d = format.parse(date);
      return convertTimezone ? Functions.convertTime(d, DCP_TIMEZONE, LOCAL_TIMEZONE) : d;
    } catch (ParseException e) {
      throw new DeliveryException("unparsable_date", date);
    }
  }

  public static String convertDateToDcpFormat(Date date) {
    return convertDateToDcpFormat(date, true);
  }

  public static String convertDateToDcpFormat(Date date, boolean convertTimezone) {
    SimpleDateFormat format = new SimpleDateFormat(DATE_FORMAT);
    return format.format(convertTimezone ? Functions.convertTime(date, LOCAL_TIMEZONE, DCP_TIMEZONE) : date);
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


}
