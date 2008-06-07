package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.ThreadFactoryWithCounter;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SubmitResponse;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;
import ru.sibinco.smsx.utils.DataSourceException;

import java.sql.Timestamp;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ArrayBlockingQueue;

/**
 * User: artem
 * Date: 04.07.2007
 */

class MessageSender {

  private static final Category log = Category.getInstance("SECRET");

  private final SecretDataSource ds;
  private final OutgoingQueue outQueue;
  private final ThreadPoolExecutor executor;

  private String serviceAddress;
  private String msgDestinationAbonentInform;
  private String msgDestinationAbonentInvitation;
  private String msgDeliveryReport;

  private volatile int rejectedTasks;


  MessageSender(SecretDataSource ds, OutgoingQueue outQueue) {
    this.ds = ds;
    this.outQueue = outQueue;
    this.executor = new ThreadPoolExecutor(3, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue(1000), new ThreadFactoryWithCounter("SecMsgSender-Executor-"));    
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

  public void setMsgDestinationAbonentInform(String msgDestinationAbonentInform) {
    this.msgDestinationAbonentInform = msgDestinationAbonentInform;
  }

  public void setMsgDestinationAbonentInvitation(String msgDestinationAbonentInvitation) {
    this.msgDestinationAbonentInvitation = msgDestinationAbonentInvitation;
  }

  public void setMsgDeliveryReport(String msgDeliveryReport) {
    this.msgDeliveryReport = msgDeliveryReport;
  }

  public void setServiceAddress(String serviceAddress) {
    this.serviceAddress = serviceAddress;
  }

  public void sendInformMessage(SecretMessage secretMessage) {
    try {
      sendMessage(serviceAddress, secretMessage.getDestinationAddress(), prepareInformMessage(secretMessage.getDestinationAddress(), secretMessage.getSourceAddress()), "smsx");
    } catch (DataSourceException e) {
      log.error("Can't send inform message", e);
    }
  }

  public void sendInvitationMessage(String destinationAddress) {
    sendMessage(serviceAddress, destinationAddress, msgDestinationAbonentInvitation, "smsx");
  }

  public void sendSecretMessage(SecretMessage message) {

    final Message outMsg = new Message();
    outMsg.setSourceAddress(message.getSourceAddress());
    outMsg.setDestinationAddress(message.getDestinationAddress());
    outMsg.setDestAddrSubunit(message.getDestAddressSubunit());
    outMsg.setMessageString(message.getMessage());
    outMsg.setConnectionName(message.getConnectionName());
    if (message.isSaveDeliveryStatus())
      outMsg.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);

    final SecretTransportObject outObj = new SecretTransportObject(message);
    outObj.setMessage(outMsg);
    try {
      outQueue.offer(outObj);
    } catch (ShutdownedException e) {
      log.error(e,e);
      return;
    }

    message.setStatus(SecretMessage.STATUS_PROCESSED);
    try {
      ds.updateMessageStatus(message);
    } catch (DataSourceException e) {
      log.error("Can't save secret message", e);
    }

    // Send notification to originator
    if (message.isNotifyOriginator())
      sendMessage(serviceAddress, message.getSourceAddress(), prepareDeliveryReport(message.getDestinationAddress(), message.getSendDate()), message.getConnectionName());
  }

  private void sendMessage(String sourceAddress, String destinationAddress, String msg, String connectionName) {
    final Message notificationMessage = new Message();
    notificationMessage.setSourceAddress(sourceAddress);
    notificationMessage.setDestinationAddress(destinationAddress);
    notificationMessage.setMessageString(msg);
    notificationMessage.setConnectionName(connectionName);
    final OutgoingObject outObj1 = new OutgoingObject();
    outObj1.setMessage(notificationMessage);
    try {
      outQueue.offer(outObj1);
    } catch (ShutdownedException e) {
      log.error(e,e);
    }
  }

  private String prepareDeliveryReport(final String toAbonent, final Timestamp sendDate) {
    return msgDeliveryReport.replaceAll("\\{to_abonent}", toAbonent).replaceAll("\\{send_date}", sendDate.toString());
  }

  private String prepareInformMessage(final String toAbonent, final String fromAbonent) throws DataSourceException {
    return msgDestinationAbonentInform;
  }


  private class SecretTransportObject extends OutgoingObject {
    private final SecretMessage msg;

    SecretTransportObject(SecretMessage msg) {
      this.msg = msg;
    }

    @Override
    public void handleResponse(PDU pdu) {
      if (msg.isSaveDeliveryStatus()) {
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
      if (msg.isSaveDeliveryStatus()) {
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
          msg.setStatus(SecretMessage.STATUS_DELIVERY_FAILED);
          ds.updateMessageStatus(msg);
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
