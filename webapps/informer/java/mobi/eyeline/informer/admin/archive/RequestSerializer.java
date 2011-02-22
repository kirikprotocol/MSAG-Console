package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.io.PrintWriter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;

/**
 * @author Aleksandr Khalitov
 */
class RequestSerializer {

  static void serialize(PrintWriter w, Delivery d, Message m) {
    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
    w.print(m.getId());
    w.print("|");
    w.print(m.getAbonent().getSimpleAddress());
    w.print("|");
    w.print(d.getId());
    w.print("|");
    w.print(d.getName());
    w.print("|");
    w.print(d.getOwner());
    w.print("|");
    w.print(m.getState());
    w.print("|");
    w.print(m.getErrorCode() == null ? " " : m.getErrorCode());
    w.print("|");
    w.print(m.getDate() != null ? df.format(m.getDate()) : " ");
    w.print("|");
    w.print(m.getText());
  }

  static void deserialize(String line, ArchiveMessage message) throws ParseException {
    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
    String[] parts = line.split("\\|", 9);
    message.setId(Long.parseLong(parts[0]));
    message.setAbonent(new Address(parts[1]));
    message.setDeliveryId(Integer.parseInt(parts[2]));
    message.setDeliveryName(parts[3]);
    message.setOwner(parts[4]);
    message.setState(MessageState.valueOf(parts[5]));
    message.setErrorCode(parts[6].equals(" ") ? null : Integer.parseInt(parts[6]));
    message.setDate(parts[7].equals(" ") ? null : df.parse(parts[7]));
    message.setText(parts[8]);
  }

  static void serialize(PrintWriter w, Delivery d, DeliveryStatistics statistics, DeliveryStatusHistory statusHistory) {

    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    w.print(d.getId());
    w.print("|");
    w.print(d.getName());
    w.print("|");
    w.print(d.getOwner());
    w.print("|");
    w.print(d.getSourceAddress().getSimpleAddress());
    w.print("|");
    w.print(df.format(d.getStartDate()));
    w.print("|");
    w.print(d.getEndDate() == null ? " " : df.format(d.getEndDate()));
    w.print("|");
    boolean first = true;
    for(Day day : d.getActiveWeekDays()) {
      if(!first) {
        w.print(',');
      }
      first = false;
      w.print(day.getDay());
    }
    w.print("|");
    w.print(d.getActivePeriodStart());
    w.print("|");
    w.print(d.getActivePeriodEnd());
    w.print("|");
    w.print(d.getPriority());
    w.print("|");
    w.print(d.isRetryOnFail());
    w.print("|");
    w.print(d.getRetryPolicy() == null || d.getRetryPolicy().length() == 0 ? " " : d.getRetryPolicy());
    w.print("|");
    w.print(d.getValidityPeriod() == null ? " " : d.getValidityPeriod());
    w.print("|");
    String p = d.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
    w.print(p == null || p.length() == 0 ? " " : p);
    w.print("|");
    p = d.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
    w.print(p == null || p.length() == 0 ? " " : p);
    w.print("|");
    w.print(d.isFlash());
    w.print("|");
    w.print(d.isTransactionMode());
    w.print("|");
    w.print(d.isUseDataSm());
    w.print("|");
    w.print(d.getDeliveryMode());
    w.print("|");
    w.print(statistics.getDeliveredMessages());
    w.print("|");
    w.print(statistics.getFailedMessages()+statistics.getExpiredMessages());
    w.print("|");
    first = true;
    for(DeliveryStatusHistory.Item h : statusHistory.getHistoryItems()) {
      if(!first) {
        w.print(',');
      }
      first = false;
      w.print(df.format(h.getDate()));
      w.print("-");
      w.print(h.getStatus());
    }
    w.println();
  }

  static void deserialize(String line, ArchiveDelivery delivery) throws ParseException {

    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    StringTokenizer tokenizer = new StringTokenizer(line, "|");

    delivery.setId(Integer.parseInt(tokenizer.nextToken()));
    delivery.setName(tokenizer.nextToken());

    delivery.setOwner(tokenizer.nextToken());

    delivery.setSourceAddress(new Address(tokenizer.nextToken()));

    delivery.setStartDate(df.parse(tokenizer.nextToken()));

    String date = tokenizer.nextToken();
    if(date.length()>1) {
      delivery.setEndDate(df.parse(date));
    }

    StringTokenizer days = new StringTokenizer(tokenizer.nextToken(), ",");
    Day[] activeDs = new Day[days.countTokens()];
    int i = 0;
    while(days.hasMoreTokens()) {
      activeDs[i] = Day.valueOf(Integer.parseInt(days.nextToken()));
      i++;
    }
    delivery.setActiveWeekDays(activeDs);

    delivery.setActivePeriodStart(new Time(tokenizer.nextToken()));
    delivery.setActivePeriodEnd(new Time(tokenizer.nextToken()));

    delivery.setPriority(Integer.parseInt(tokenizer.nextToken()));

    delivery.setRetryOnFail(Boolean.valueOf(tokenizer.nextToken()));

    String p = tokenizer.nextToken();

    if(p.length()>1) {
      delivery.setRetryPolicy(p);
    }

    String t = tokenizer.nextToken();

    if(t != null) {
      delivery.setValidityPeriod(new Time(t));
    }

    String a = tokenizer.nextToken();

    if(a.length()>1) {
      delivery.setEmailNotification(a);
    }

    a = tokenizer.nextToken();
    if(a.length()>1) {
      delivery.setSmsNotification(new Address(a));
    }

    delivery.setFlash(Boolean.valueOf(tokenizer.nextToken()));
    delivery.setTransactionMode(Boolean.valueOf(tokenizer.nextToken()));

    delivery.setUseDataSm(Boolean.valueOf(tokenizer.nextToken()));
    delivery.setDeliveryMode(DeliveryMode.valueOf(tokenizer.nextToken()));

    delivery.setDelivered(Integer.parseInt(tokenizer.nextToken()));
    delivery.setFailed(Integer.parseInt(tokenizer.nextToken()));

    StringTokenizer hTok = new StringTokenizer(tokenizer.nextToken(), ",");

    List<DeliveryStatusHistory.Item> items = new ArrayList<DeliveryStatusHistory.Item>(hTok.countTokens());

    while(hTok.hasMoreTokens()) {
      String[] ss = hTok.nextToken().split("-",2);
      items.add(new DeliveryStatusHistory.Item(df.parse(ss[0]), DeliveryStatus.valueOf(ss[1])));
    }

    delivery.setHistory(items);

  }









}
