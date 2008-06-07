package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.ThreadFactoryWithCounter;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SubmitResponse;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderDataSource;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderMessage;
import ru.sibinco.smsx.utils.DataSourceException;

import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ArrayBlockingQueue;

/**
 * User: artem
 * Date: 06.07.2007
 */

class MessageSender {

  private static final Category log = Category.getInstance("SENDER");

  private final OutgoingQueue outQueue;
  private final SenderDataSource ds;
  private final ThreadPoolExecutor executor;

  private volatile int rejectedTasks;

  MessageSender(OutgoingQueue outQueue, SenderDataSource ds) {
    this.outQueue = outQueue;
    this.ds = ds;
    this.executor = new ThreadPoolExecutor(3, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue(1000), new ThreadFactoryWithCounter("SenderMsgSender-Executor-"));
  }

  public void sendMessage(SenderMessage message) {
    final Message msg = new Message();
    msg.setSourceAddress(message.getSourceAddress());
    msg.setDestinationAddress(message.getDestinationAddress());
    msg.setMessageString(message.getMessage());
    msg.setDestAddrSubunit(message.getDestAddrSubunit());
    msg.setConnectionName(message.getConnectionName());
    if (message.isStorable())
      msg.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);

    final SenderSMPPTransportObject outObj = new SenderSMPPTransportObject(message);
    outObj.setMessage(msg);
    try {
      outQueue.offer(outObj);
    } catch (ShutdownedException e) {
      log.error(e,e);
    }

    message.setStatus(SenderMessage.STATUS_PROCESSED);
  }

  public int getExecutorActiveCount() {
    return executor.getActiveCount();
  }

  public int getExecutorPoolSize() {
    return executor.getPoolSize();
  }

  public int getExecutorMaxPoolSize() {
    return executor.getMaximumPoolSize();
  }

  public void setExecutorMaxPoolSize(int size) {
    executor.setMaximumPoolSize(size);
  }

  public int getExecutorRejectedTasks() {
    return rejectedTasks;
  }


  private class SenderSMPPTransportObject extends OutgoingObject {
    private final SenderMessage msg;

    SenderSMPPTransportObject(SenderMessage senderMessage) {
      this.msg = senderMessage;
    }

    @Override
    public void handleResponse(PDU pdu) {
      if (msg.isStorable()) {
        if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR) {
          try {
            executor.execute(new UpdateMessageStatusTask());
          } catch (Throwable e) {
            log.error("Can't execute UpdateMessageStatusTask", e);
            rejectedTasks++;
          }
        } else if (pdu.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR) {
          try {
            executor.execute(new UpdateSMPPIdTask(Long.parseLong(((SubmitResponse)pdu).getMessageId())));
          } catch (Throwable e) {
            log.error("Can't execute UpdateSMPPIdTask", e);
            rejectedTasks++;
          }
        }
      }
    }

    @Override
    public void handleSendError() {
      if (msg.isStorable()) {
        try {
          executor.execute(new UpdateMessageStatusTask());
        } catch (Throwable e) {
          log.error("Can't execute UpdateMessageStatusTask", e);
          rejectedTasks++;
        }
      }
    }

    private class UpdateMessageStatusTask implements Runnable {
      public void run() {
        try {
          msg.setStatus(SenderMessage.STATUS_DELIVERY_FAILED);
          ds.saveSenderMessage(msg);
        } catch (Throwable e) {
          log.error("Update msg status err:",e);
        }
      }
    }

    private class UpdateSMPPIdTask implements Runnable {
      private final long messageId;

      private UpdateSMPPIdTask(long messageId) {
        this.messageId = messageId;
      }

      public void run() {
        msg.setSmppId(messageId);
        try {
          ds.updateMessageSmppId(msg);
        } catch (Throwable e) {
          log.error("Can't update smpp id: ", e);
        }
      }
    }
  }

}
