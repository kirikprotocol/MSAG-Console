package ru.sibinco.calendarsme.network.sar;

import ru.aurorisoft.smpp.Message;

import java.util.*;

public class Assembler {

  private final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(Assembler.class);

  private final Map cache = new HashMap();
  private final List tqueue = new ArrayList();
  protected final long timeout;

  public Assembler(long timeout) {
    this.timeout = timeout;
  }

  public synchronized Message put(Message msg) {
    Log.debug("Assembler.put() called. Mgs " + (msg.hasUDH() ? "has" : "hasn't") + " UDH. " + "Mgs " + (msg.hasConcatInfo() ? "has" : "hasn't") + " ConcatInfo.");
    if (msg.hasConcatInfo()) {
      Key key = new Key(msg);
      Log.debug("Keep MSG " + key.toString());
      MessageKeeper keeper = (MessageKeeper) cache.get(key);
      if (keeper == null) {
        keeper = new MessageKeeper(msg.getConcatInfo().getTotal());
        cache.put(key, keeper);
        tqueue.add(new Deadline(System.currentTimeMillis() + timeout, key));
        Log.debug("MSG is part of existed one.");
      }
      keeper.add(msg);
      Message result = null;
      if (keeper.isComplete()) {
        cache.remove(key);
        result = keeper.assemble();
        Log.debug("MSG is last part of existed one. Assemble message.");
      }
      return result;
    } else if (msg.hasSarMsgRefNum() && msg.hasSarSegmentSeqnum() && msg.hasSarTotalSegments()) {
      Key key = new Key(msg);
      Log.debug("Keep MSG " + key.toString());
      MessageKeeper keeper = (MessageKeeper) cache.get(key);
      if (keeper == null) {
        keeper = new MessageKeeper(msg.getSarTotalSegments());
        cache.put(key, keeper);
        tqueue.add(new Deadline(System.currentTimeMillis() + timeout, key));
        Log.debug("MSG is part of existed one.");
      }
      keeper.add(msg);
      Message result = null;
      if (keeper.isComplete()) {
        cache.remove(key);
        result = keeper.assemble();
        Log.debug("MSG is last part of existed one. Assemble message.");
      }
      return result;
    } else
      return msg;
  }

  public synchronized List removeObsoletes() {
    long now = System.currentTimeMillis();
    List obsoletes = new ArrayList();
    for (ListIterator iterator = tqueue.listIterator(); iterator.hasNext();) {
      Deadline deadline = (Deadline) iterator.next();
      if (deadline.deadline < now) {
        MessageKeeper keeper = (MessageKeeper) cache.remove(deadline.key);
        if (keeper != null) obsoletes.add(keeper);
        iterator.remove();
      } else
        break;
    }
    if (obsoletes.size() == 0)
      return null;
    else {
      List msgs = new ArrayList();
      for (ListIterator iterator = obsoletes.listIterator(); iterator.hasNext();) {
        MessageKeeper keeper = (MessageKeeper) iterator.next();
        Message msg = keeper.assemble();
        if (msg != null)
          msgs.add(msg);
      }
      if (msgs.size() == 0)
        return null;
      else
        return msgs;
    }
  }

  private class Key {
    private final String oa;
    private final String da;
    private final int mr;

    public Key(final Message msg) {
      oa = new String(msg.getSourceAddress());
      da = new String(msg.getDestinationAddress());
      if (msg.hasConcatInfo())
        mr = msg.getConcatInfo().getMr();
      else
        mr = msg.getSarMsgRefNum();
    }

    public boolean equals(Object o) {
      if (this == o) return true;
      if (!(o instanceof Key)) return false;
      final Key key = (Key) o;
      return mr == key.mr && da.equals(key.da) && oa.equals(key.oa);
    }

    public int hashCode() {
      int result;
      result = oa.hashCode();
      result = 29 * result + da.hashCode();
      result = 29 * result + mr;
      return result;
    }

    public String toString() {
      return "sa=" + oa + "; da=" + da + ";mr=" + mr;
    }
  }

  private class Deadline {
    private final long deadline;
    private final Key key;

    public Deadline(long deadline, Key key) {
      this.deadline = deadline;
      this.key = key;
    }
  }

  private class MessageKeeper {
    private final Message[] msgs;

    public MessageKeeper(int size) {
      msgs = new Message[size];
    }

    public void add(final Message msg) {
      if (msg.hasConcatInfo())
        msgs[msg.getConcatInfo().getSeq() - 1] = msg;
      else
        msgs[msg.getSarSegmentSeqnum() - 1] = msg;
    }

    public boolean isComplete() {
      for (int i = 0; i < msgs.length; i++) {
        if (msgs[i] == null) return false;
      }
      return true;
    }

    public Message[] toArray() {
      return msgs;
    }

    public Message assemble() {
      if (!isComplete())
        return assembleNonCompleted();
      StringBuffer sb = new StringBuffer();
      for (int i = 0; i < msgs.length; i++) {
        if (msgs[i] != null)
          sb.append(msgs[i].getMessageString());
      }
      if (msgs[0] == null) {
        Log.error("Impossible situation! Connect with support immediately!");
        return null;
      }
      msgs[0].setEsmClass((byte) 0);
      msgs[0].setMessageString(sb.toString());
      Log.debug("Assembled MSG: Address #" + msgs[0].getDestinationAddress() + "; Abonent #" + msgs[0].getSourceAddress() + "; Encoding=" + msgs[0].getEncoding() + "(" + msgs[0].getEncodingString() + "); Text=" + msgs[0].getMessageString());
      return msgs[0];
    }

    private Message assembleNonCompleted() {
      StringBuffer sb = new StringBuffer();
      Message msg = null;
      for (int i = 0; i < msgs.length; i++) {
        if (msgs[i] != null) {
          sb.append(msgs[i].getMessageString());
          if (msg == null) msg = msgs[i];
        } else
          sb.append(" ... ");
      }
      if (msg == null) {
        Log.error("Impossible situation! Connect with support immediately!");
        return null;
      }
      msg.setEsmClass((byte) 0);
      msg.setMessageString(sb.toString().trim());
      Log.debug("Assembled not full MSG: Address #" + msg.getDestinationAddress() + "; Abonent #" + msg.getSourceAddress() + "; Encoding=" + msg.getEncoding() + "(" + msg.getEncodingString() + "); Text=" + msg.getMessageString());
      return msg;
    }
  }
}
