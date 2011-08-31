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
    w.print(d.getName().replace("|","\\|"));
    w.print("|");
    w.print(d.getOwner());
    w.print("|");
    w.print(m.getState());
    w.print("|");
    w.print(m.getErrorCode() == null ? " " : m.getErrorCode());
    w.print("|");
    w.print(m.getDate() != null ? df.format(m.getDate()) : " ");
    w.print("|");
    w.print(m.getText().replace("\n","\\n").replace("|","\\|"));
    w.println();
  }

  static void deserialize(String line, ArchiveMessage message) throws ParseException {
    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    StringTokenizer tokenizer = new StringTokenizer(line, "|");

    message.setId(Long.parseLong(nextToken(tokenizer)));
    message.setAbonent(new Address(nextToken(tokenizer)));
    message.setDeliveryId(Integer.parseInt(nextToken(tokenizer)));
    message.setDeliveryName(nextToken(tokenizer));
    message.setOwner(nextToken(tokenizer));
    message.setState(MessageState.valueOf(nextToken(tokenizer)));
    String s = nextToken(tokenizer);
    message.setErrorCode(s.equals(" ") ? null : Integer.parseInt(s));
    s = nextToken(tokenizer);
    message.setDate(s.equals(" ") ? null : df.parse(s));
    message.setText(nextToken(tokenizer));
  }

  static void serialize(PrintWriter w, Delivery d) {

    final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    w.print(d.getId());
    w.print("|");
    w.print(d.getName().replace("|","\\|"));
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

    delivery.setId(Integer.parseInt(nextToken(tokenizer)));
    delivery.setName(nextToken(tokenizer));

    delivery.setOwner(nextToken(tokenizer));

    delivery.setStartDate(df.parse(nextToken(tokenizer)));

    String date = nextToken(tokenizer);
    if(date.length()>1) {
      delivery.setEndDate(df.parse(date));
    }

  }

  private static String nextToken(StringTokenizer tokenizer) {
    String res = tokenizer.nextToken();
    while(res.endsWith("\\") && tokenizer.hasMoreTokens()) {
      res=res.substring(0,res.length()-1)+'|'+tokenizer.nextToken();
    }
    return res.replace("\\n","\n");
  }









}
