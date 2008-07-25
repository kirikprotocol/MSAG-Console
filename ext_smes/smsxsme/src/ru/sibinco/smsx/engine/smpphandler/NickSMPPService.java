package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.smpp.IncomingObject;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.CommandObserver;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.nick.commands.NickRegisterCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickSendMessageCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickUnregisterCmd;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class NickSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance("NICK SMPP");

  public boolean serve(SMPPRequest smppRequest) {

    String type = smppRequest.getParameter("type");
    if (type == null) {
      log.error("'type' property is empty in request");
      return false;
    }

    final IncomingObject inObj = smppRequest.getInObj();
    final String sourceAddress = inObj.getMessage().getSourceAddress();

    try {

      if (type.equalsIgnoreCase("register")) {
        registerNick(sourceAddress, smppRequest.getParameter("nick"), inObj);
      } else if (type.equalsIgnoreCase("unregister")) {
        unregisterNick(sourceAddress, inObj);
      } else if (type.equalsIgnoreCase("send")) {
        sendNickMessage(sourceAddress, smppRequest.getParameter("destination_address"), smppRequest.getParameter("message"), inObj);
      } else {
        log.error("Unknown msg type parameter: " + type);
        return false;
      }

      return true;
    } catch (Throwable e) {
      log.error(e,e);
      return false;
    }
  }

  private static void unregisterNick(String sourceAddress, final IncomingObject inObj) {
    final NickUnregisterCmd cmd = new NickUnregisterCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setSourceId(AsyncCommand.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(AsyncCommand command) {
        final NickUnregisterCmd cmd = (NickUnregisterCmd)command;
        try {
          switch (cmd.getStatus()) {
            case NickUnregisterCmd.STATUS_DELIVERED:
              inObj.respond(cmd.getSmppStatus());
              break;
            case NickUnregisterCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    Services.getInstance().getNickService().execute(cmd);
  }

  private static void registerNick(String sourceAddress, String nick, final IncomingObject inObj) {
    final NickRegisterCmd cmd = new NickRegisterCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setNick(nick);
    cmd.setSourceId(AsyncCommand.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(AsyncCommand command) {
        final NickRegisterCmd cmd = (NickRegisterCmd)command;
        try {
          switch (cmd.getStatus()) {
            case NickRegisterCmd.STATUS_INVALID_ABONENT_ADDRESS:
            case NickRegisterCmd.STATUS_INVALID_NICK:
              log.error("Invalid nick or abonent address");
              inObj.respond(Data.ESME_RX_P_APPN);
              break;
            case NickRegisterCmd.STATUS_DELIVERED:
              inObj.respond(cmd.getSmppStatus());
              break;
            case NickRegisterCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    Services.getInstance().getNickService().execute(cmd);
  }

  private static void sendNickMessage(String sourceAddress, String destinationAddress, String message, final IncomingObject inObj) {
    final NickSendMessageCmd cmd = new NickSendMessageCmd();
    cmd.setSourceAddress(sourceAddress);
    cmd.setDestinationAddress(destinationAddress);
    cmd.setMessage(message);
    cmd.setMscAddress(inObj.getMessage().getMscAddress());
    cmd.setImsi(inObj.getMessage().getImsi());
    cmd.setSourceId(AsyncCommand.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(AsyncCommand command) {
        final NickSendMessageCmd cmd = (NickSendMessageCmd)command;
        try {
          switch (cmd.getStatus()) {
            case NickSendMessageCmd.STATUS_INVALID_DESTINATION_ADDRESS:
            case NickSendMessageCmd.STATUS_INVALID_SOURCE_ADDRESS:
              log.error("Invalid src or dst addr");
              inObj.respond(Data.ESME_RX_P_APPN);
              break;
            case NickSendMessageCmd.STATUS_DELIVERED:
              inObj.respond(cmd.getSmppStatus());
              break;
            case NickSendMessageCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    Services.getInstance().getNickService().execute(cmd);
  }
}
