package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.util.Address;

import java.io.PrintWriter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
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
    w.println();
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

  static void serialize(PrintWriter w, Delivery d) {

    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    w.print(d.getId());
    w.print("|");
    w.print(d.getName());
    w.print("|");
    w.print(d.getOwner());
    w.print("|");
    w.print(df.format(d.getStartDate()));
    w.print("|");
    w.print(d.getEndDate() == null ? " " : df.format(d.getEndDate()));
    w.println();
  }

  static void deserialize(String line, ArchiveDelivery delivery) throws ParseException {

    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    StringTokenizer tokenizer = new StringTokenizer(line, "|");

    delivery.setId(Integer.parseInt(tokenizer.nextToken()));
    delivery.setName(tokenizer.nextToken());

    delivery.setOwner(tokenizer.nextToken());

    delivery.setStartDate(df.parse(tokenizer.nextToken()));

    String date = tokenizer.nextToken();
    if(date.length()>1) {
      delivery.setEndDate(df.parse(date));
    }

  }









}
