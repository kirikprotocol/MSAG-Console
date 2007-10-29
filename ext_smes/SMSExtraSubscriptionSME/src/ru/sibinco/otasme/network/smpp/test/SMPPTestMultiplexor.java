package ru.sibinco.otasme.network.smpp.test;

import com.eyeline.sme.utils.worker.IterativeWorker;
import com.eyeline.sme.utils.queue.blocking.BlockingQueue;
import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;

/**
 * User: artem
 * Date: Oct 8, 2007
 */

public class SMPPTestMultiplexor extends SMPPTestTransportMultiplexor {

  private final Worker worker = new Worker("+79607891901", null);
  private int cnt = 0;
  private long start = System.currentTimeMillis();

  public void connect() {
    worker.start();
  }

  public void sendMessage(Message message1, String connectorName, boolean flag) throws SMPPException {
//    responseListener.handleResponse(new MySubmitResponse(message1, Data.ESME_RSYSERR, Message.STATUS_CLASS_PERM_ERROR));
    cnt++;
    responseListener.handleResponse(new MySubmitResponse(message1));
    if (message1.getType() == Message.TYPE_WTS_REQUEST) {
      final Message message = new Message();
      message.setType(Message.TYPE_WTS_REQUEST);
      message.setSourceAddress(message1.getDestinationAddress());
      message.setWtsRequestReference(message1.getWtsRequestReference());
      message.setWtsOperationCode(Message.WTS_OPERATION_CODE_AACK);
      if (Math.random() > 0.5)
        message.setWTSErrorCode(20);
      else
        message.setWTSErrorCode(20);
      handleMessage(message);
    }
    if (cnt == 10000) {
      System.out.println("Work Speed = " + 10000 * 1000 / (System.currentTimeMillis() - start));
    }
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
        wait(5000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
      System.out.println("start!!!");
      start = System.currentTimeMillis();
      cnt = 0;
      Message msg;
      for (int i=10000; i < 20000; i++) {
        msg = new Message();
        msg.setSourceAddress("+791378" + i);
        msg.setDestinationAddress("5707");
        msg.setMessageString("ON");
        msg.setConnectionName("mar");
        handleMessage(msg);

        try {
          wait(100000);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }

      try {
        wait(10000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }

    protected void stopCurrentWork() {

    }
  }
}
