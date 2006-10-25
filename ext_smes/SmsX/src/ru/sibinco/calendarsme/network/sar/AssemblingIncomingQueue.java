package ru.sibinco.calendarsme.network.sar;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.calendarsme.network.IncomingObject;
import ru.sibinco.calendarsme.network.IncomingQueue;
import ru.sibinco.calendarsme.network.QueueOverflowException;
import ru.sibinco.calendarsme.utils.Utils;

import java.util.*;

public class AssemblingIncomingQueue extends IncomingQueue {

  private final Assembler assembler;
  private long lastCheckObsoletes = -1;

  public AssemblingIncomingQueue(final Properties config) {
    super(config);
    this.assembler = new Assembler(Utils.loadLong(config, "incoming.queue.assembling.timeout"));
  }

  public void addIncomingObject(IncomingObject obj) {
    if (obj == null) return;
    Message msg = assembler.put(obj.getMessage());
    if (msg != null)
      super.addIncomingObject(new IncomingObject(msg));
  }

  public IncomingObject getIncomingObject() {
    if (System.currentTimeMillis() - assembler.timeout > lastCheckObsoletes) {
      List msgs = assembler.removeObsoletes();
      if (msgs != null) {
        for (ListIterator iterator = msgs.listIterator(); iterator.hasNext();) {
          super.addIncomingObject(new IncomingObject((Message) iterator.next()));
        }
      }
    }
    return super.getIncomingObject();
  }
}
