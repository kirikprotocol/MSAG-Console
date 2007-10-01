package ru.sibinco.smsx.test.testcase.calendar;

import com.eyeline.sme.utils.queue.blocking.BlockingQueue;
import com.eyeline.sme.utils.worker.IterativeWorker;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.test.testcase.SMPPTestTransportMultiplexor;

/**
 * User: artem
 * Date: 11.07.2007
 */

public class CalendarMultiplexor extends SMPPTestTransportMultiplexor {

  private final Worker worker = new Worker("+79607891901", null);

  public void connect() {
    worker.start();
  }


  private class Worker extends IterativeWorker {

    private final String sourceAddress;
    private final BlockingQueue inQueue;
    private final static int count = 10000;

    private Worker(String sourceAddress, BlockingQueue inQueue) {
      super(null);
      this.sourceAddress = sourceAddress;
      this.inQueue = inQueue;
    }

    public synchronized void iterativeWork() {

      try {
        wait(20000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }

      long start = System.currentTimeMillis();
      Message msg;
      for (int i=0; i < count; i++) {
        msg = new Message();
        msg.setSourceAddress(sourceAddress);
        msg.setDestinationAddress("+79139023974");
        msg.setMessageString("AFT 1 gg");
        handleMessage(msg);
      }
      System.out.println("Calendar perf= " + count * 1000 / (System.currentTimeMillis() - start));

      try {
        wait(30000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }

    protected void stopCurrentWork() {
      
    }
  }
}