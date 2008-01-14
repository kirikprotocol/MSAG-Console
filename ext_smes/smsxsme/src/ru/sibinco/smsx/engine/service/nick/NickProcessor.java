package ru.sibinco.smsx.engine.service.nick;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.nick.commands.NickRegisterCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickSendMessageCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickUnregisterCmd;
import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

class NickProcessor implements NickRegisterCmd.Receiver, NickSendMessageCmd.Receiver, NickUnregisterCmd.Receiver {

  private static final Category log = Category.getInstance("NICK");

  private final NickServiceConfig config;
  private final SMPPOutgoingQueue outQueue;

  public NickProcessor(NickServiceConfig config, SMPPOutgoingQueue outQueue) {
    this.config = config;
    this.outQueue = outQueue;
  }

  public void execute(final NickRegisterCmd cmd) {
    if (cmd == null)
      return;

    try {
      if (log.isInfoEnabled())
        log.info("Reg nick req: abonent=" + cmd.getAbonentAddress());

      // Check source address
      if (cmd.getAbonentAddress() == null || cmd.getAbonentAddress().length() == 0) {
        cmd.update(NickRegisterCmd.STATUS_INVALID_ABONENT_ADDRESS);
        return;
      }

      // Check nick
      if (cmd.getNick() == null || cmd.getNick().length() == 0) {
        cmd.update(NickRegisterCmd.STATUS_INVALID_NICK);
        return;
      }

      final Message msg = new Message();
      msg.setSourceAddress(cmd.getAbonentAddress());
      msg.setDestinationAddress(config.getProfilerAddress());
      msg.setMessageString("SETNICK " + cmd.getNick());

      final SMPPTransportObject outObj = new SMPPTransportObject() {
        public void handleResponse(PDU response) {
          cmd.setSmppStatus(response.getStatus());
          cmd.update(NickRegisterCmd.STATUS_DELIVERED);
        }

        public void handleSendError() {
          cmd.update(NickRegisterCmd.STATUS_SYSTEM_ERROR);
        }
      };

      outObj.setOutgoingMessage(msg);

      cmd.update(NickRegisterCmd.STATUS_MESSAGE_SENDED);
      outQueue.addOutgoingObject(outObj);

    } catch (Throwable e) {
      log.error("Reg nick erro for " + cmd.getAbonentAddress(), e);
      cmd.update(NickRegisterCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(final NickSendMessageCmd cmd) {
    if (cmd == null)
      return;

    try {
      if (log.isInfoEnabled())
        log.info("Send nick req: srcAddr=" + cmd.getSourceAddress() + "; dstAddr=" + cmd.getDestinationAddress());

      // Check source address
      if (cmd.getSourceAddress() == null || cmd.getSourceAddress().length() == 0) {
        cmd.update(NickSendMessageCmd.STATUS_INVALID_SOURCE_ADDRESS);
        return;
      }

      // Check destination address
      if (cmd.getDestinationAddress() == null || cmd.getDestinationAddress().length() == 0) {
        cmd.update(NickSendMessageCmd.STATUS_INVALID_DESTINATION_ADDRESS);
        return;
      }

      final Message msg = new Message();
      msg.setSourceAddress(cmd.getSourceAddress());
      msg.setDestinationAddress(cmd.getDestinationAddress());
      msg.setMessageString(cmd.getMessage());
      msg.setMscAddress(cmd.getMscAddress());
      msg.setImsi(cmd.getImsi());
      msg.setPrivacyIndicator((byte)2);

      final SMPPTransportObject outObj = new SMPPTransportObject() {
        public void handleResponse(PDU response) {
          cmd.setSmppStatus(response.getStatus());
          cmd.update(NickSendMessageCmd.STATUS_DELIVERED);
        }

        public void handleSendError() {
          cmd.update(NickSendMessageCmd.STATUS_SYSTEM_ERROR);
        }
      };

      outObj.setOutgoingMessage(msg);

      cmd.update(NickSendMessageCmd.STATUS_MESSAGE_SENDED);
      outQueue.addOutgoingObject(outObj);

    } catch (Throwable e) {
      log.error("Send nick err for " + cmd.getSourceAddress(), e);
      cmd.update(NickSendMessageCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(final NickUnregisterCmd cmd) {
    if (cmd == null)
      return;

    try {
      if (log.isInfoEnabled())
        log.info("Unreg nick req: srcAddr=" + cmd.getAbonentAddress());

      final Message msg = new Message();
      msg.setSourceAddress(cmd.getAbonentAddress());
      msg.setDestinationAddress(config.getProfilerAddress());
      msg.setMessageString("CLEARNICK");

      final SMPPTransportObject outObj = new SMPPTransportObject() {
        public void handleResponse(PDU response) {
          cmd.setSmppStatus(response.getStatus());
          cmd.update(NickUnregisterCmd.STATUS_DELIVERED);
        }

        public void handleSendError() {
          cmd.update(NickUnregisterCmd.STATUS_SYSTEM_ERROR);
        }
      };

      outObj.setOutgoingMessage(msg);

      cmd.update(NickRegisterCmd.STATUS_MESSAGE_SENDED);
      outQueue.addOutgoingObject(outObj);

    } catch (Throwable e) {
      log.error("Unreg nick err for " + cmd.getAbonentAddress());
      cmd.update(NickSendMessageCmd.STATUS_SYSTEM_ERROR);
    }
  }

}
