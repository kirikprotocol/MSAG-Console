package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.smpp.IncomingObject;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandObserver;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.secret.commands.*;

import java.util.Properties;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class SecretSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance("SECRET SMPP");

  private String serviceAddress;
  private String msgSourceAbonentNotRegistered;
  private String msgSourceAbonentAlreadyRegistered;
  private String msgUnregisterOk;
  private String msgRegisterOk;
  private String msgChangePwdOk;
  private String msgInvalidPassword;
  private String msgNoMessages;

  protected void init(Properties props) throws SMPPServiceException {
    try {
      final PropertiesConfig config = new PropertiesConfig(props);

      serviceAddress = config.getString("service.address");
      msgSourceAbonentNotRegistered = config.getString("source.abonent.not.registered");
      msgSourceAbonentAlreadyRegistered = config.getString("source.abonent.already.registered");
      msgUnregisterOk = config.getString("unregister.ok");
      msgRegisterOk = config.getString("register.ok");
      msgChangePwdOk = config.getString("change.password.ok");
      msgInvalidPassword = config.getString("invalid.password");
      msgNoMessages = config.getString("no.messages");

    } catch (ConfigException e) {
      throw new SMPPServiceException(e);
    }
  }

  public boolean serve(SMPPRequest smppRequest) {

    String type = smppRequest.getParameter("type");
    if (type == null) {
      log.error("'type' property is empty in request");
      return false;
    }

    final IncomingObject inObj = smppRequest.getInObj();

    try {

      if (type.equalsIgnoreCase("receipt")) { // Handle receipt

        final long msgId = Long.parseLong(inObj.getMessage().getReceiptedMessageId());
        final boolean delivered = inObj.getMessage().getMessageState() == Message.MSG_STATE_DELIVERED;

        final SecretHandleReceiptCmd cmd = new SecretHandleReceiptCmd();
        cmd.setSmppMessageId(msgId);
        cmd.setDelivered(delivered);

        if (ServiceManager.getInstance().getSecretService().execute(cmd)) {
          inObj.respond(Data.ESME_ROK);
          return true;
        }

      } else { // Handle usual message

        final String sourceAddress = inObj.getMessage().getSourceAddress();
        final String destinationAddress = inObj.getMessage().getDestinationAddress();

        if (type.equalsIgnoreCase("on")) {
          handleOnRequest(sourceAddress, smppRequest.getParameter("password").trim(), inObj);
        } else if (type.equalsIgnoreCase("off")) {
          handleOffRequest(sourceAddress, inObj);
        } else if (type.equalsIgnoreCase("change_pwd")) {
          handleChangePasswordRequest(sourceAddress, smppRequest.getParameter("old_password").trim(), smppRequest.getParameter("new_password").trim(), inObj);
        } else if (type.equalsIgnoreCase("msg") && !destinationAddress.equalsIgnoreCase(serviceAddress)) {
          handleSecretMessage(sourceAddress, destinationAddress, smppRequest.getParameter("message"), inObj);
        } else if (type.equalsIgnoreCase("get")) {
          handleGetRequest(sourceAddress, smppRequest.getParameter("password").trim(), inObj);
        } else {
          log.error("Unknown msg type parameter: " + type);
          return false;
        }

        return true;
      }

      return false;
    } catch (Throwable e) {
      log.error(e,e);
      return false;
    }
  }

  private void handleOffRequest(final String sourceAddress, final IncomingObject inObj) {
    final SecretUnregisterAbonentCmd cmd = new SecretUnregisterAbonentCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretUnregisterAbonentCmd cmd = (SecretUnregisterAbonentCmd)command;
        try {
          switch (cmd.getStatus()) {
            case SecretUnregisterAbonentCmd.STATUS_SUCCESS:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgUnregisterOk);
              break;
            case SecretUnregisterAbonentCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
              break;
            case SecretUnregisterAbonentCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;
            default:
              inObj.respond(Data.ESME_ROK);
              log.error("Unknown response code for Off request: " + cmd.getStatus());
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }



  private void handleOnRequest(final String sourceAddress, String password, final IncomingObject inObj) {
    final SecretRegisterAbonentCmd cmd = new SecretRegisterAbonentCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setPassword(password);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretRegisterAbonentCmd cmd = (SecretRegisterAbonentCmd)command;
        try {
          switch (cmd.getStatus()) {
            case SecretRegisterAbonentCmd.STATUS_SUCCESS:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgRegisterOk);
              break;
            case SecretRegisterAbonentCmd.STATUS_SOURCE_ABONENT_ALREADY_REGISTERED:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgSourceAbonentAlreadyRegistered.replaceAll("\\{abonent}", sourceAddress));
              break;
            case SecretRegisterAbonentCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;
            default:
              inObj.respond(Data.ESME_ROK);
              log.error("Unknown response code for On request: " + cmd.getStatus());
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }

  private void handleChangePasswordRequest(final String sourceAddress, String oldPwd, String newPwd, final IncomingObject inObj) {
    final SecretChangePasswordCmd cmd = new SecretChangePasswordCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setOldPassword(oldPwd);
    cmd.setNewPassword(newPwd);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretChangePasswordCmd cmd = (SecretChangePasswordCmd)command;
        try {
          switch (cmd.getStatus()) {
            case SecretChangePasswordCmd.STATUS_SUCCESS:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgChangePwdOk);
              break;
            case SecretChangePasswordCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
              break;
            case SecretChangePasswordCmd.STATUS_INVALID_PASSWORD:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgInvalidPassword);
              break;
            case SecretChangePasswordCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;
            default:
              inObj.respond(Data.ESME_ROK);
              log.error("Unknown response code for ChPwd request: " + cmd.getStatus());
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }

  private static void handleSecretMessage(final String sourceAddress, final String destinationAddress, String message, final IncomingObject inObj) {
    final SecretSendMessageCmd cmd = new SecretSendMessageCmd();
    cmd.setSourceAddress(sourceAddress);
    cmd.setDestinationAddress(destinationAddress);
    cmd.setMessage(message);
    cmd.setDestAddressSubunit(inObj.getMessage().getDestAddrSubunit());
    cmd.setSaveDeliveryStatus(false);
    cmd.setNotifyOriginator(true);
    cmd.setSourceId(Command.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver(){
      public void update(Command command) {
        final SecretSendMessageCmd cmd = (SecretSendMessageCmd)command;
        try {
          switch (cmd.getStatus()) {
            case SecretSendMessageCmd.STATUS_SUCCESS:
            case SecretSendMessageCmd.STATUS_DESTINATION_ABONENT_NOT_REGISTERED:
              inObj.respond(Data.ESME_ROK);
              break;

            case SecretSendMessageCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;

            case SecretSendMessageCmd.STATUS_DESTINATION_ADDRESS_IS_NOT_ALLOWED:
              inObj.respond(Data.ESME_RINVDSTADR);
              break;

            case SecretSendMessageCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;

            default:
              inObj.respond(Data.ESME_ROK);
              log.error("Unknown response code for SecMsg request: " + cmd.getStatus());
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }


  private void handleGetRequest(final String sourceAddress, String password, final IncomingObject inObj) {
    final SecretGetMessagesCmd cmd = new SecretGetMessagesCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setPassword(password);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretGetMessagesCmd cmd = (SecretGetMessagesCmd)command;
        try {
          switch (cmd.getStatus()) {
            case SecretGetMessagesCmd.STATUS_SUCCESS:
              inObj.respond(Data.ESME_ROK);
              break;

            case SecretGetMessagesCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
              break;

            case SecretGetMessagesCmd.STATUS_INVALID_PASSWORD:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgInvalidPassword);
              break;

            case SecretGetMessagesCmd.STATUS_NO_MESSAGES:
              inObj.respond(Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgNoMessages);
              break;

            case SecretGetMessagesCmd.STATUS_SYSTEM_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;

            default:
              inObj.respond(Data.ESME_ROK);
              log.error("Unknown response code for SecMsg request: " + cmd.getStatus());
          }
        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }
}
