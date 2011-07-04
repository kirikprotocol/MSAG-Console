package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.smpp.IncomingObject;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
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
    super.init(props);
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
        cmd.setUmr(inObj.getMessage().getUserMessageReference());

        if (Services.getInstance().getSecretService().execute(cmd)) {
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

  private void handleOffRequest(final String sourceAddress, final IncomingObject inObj) throws SMPPException {
    final SecretUnregisterAbonentCmd cmd = new SecretUnregisterAbonentCmd();
    cmd.setAbonentAddress(sourceAddress);
    try {
      Services.getInstance().getSecretService().execute(cmd);
      inObj.respond(Data.ESME_ROK);
      reply(inObj.getMessage(), serviceAddress, msgUnregisterOk);
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case SecretUnregisterAbonentCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
          break;
        case SecretUnregisterAbonentCmd.ERR_SYS_ERROR:
          inObj.respond(Data.ESME_RX_P_APPN);
          break;
        default:
          inObj.respond(Data.ESME_ROK);
          log.error("Unknown response code for Off request: " + e.getErrCode());
      }
    }
  }



  private void handleOnRequest(final String sourceAddress, String password, final IncomingObject inObj) throws SMPPException {
    final SecretRegisterAbonentCmd cmd = new SecretRegisterAbonentCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setPassword(password);

    try {
      Services.getInstance().getSecretService().execute(cmd);
      inObj.respond(Data.ESME_ROK);
      reply(inObj.getMessage(), serviceAddress, msgRegisterOk);
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case SecretRegisterAbonentCmd.ERR_SOURCE_ABONENT_ALREADY_REGISTERED:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgSourceAbonentAlreadyRegistered.replaceAll("\\{abonent}", sourceAddress));
          break;
        case SecretRegisterAbonentCmd.ERR_SYS_ERROR:
          inObj.respond(Data.ESME_RX_P_APPN);
          break;
        default:
          inObj.respond(Data.ESME_ROK);
          log.error("Unknown response code for On request: " + e.getErrCode());
      }
    }
  }

  private void handleChangePasswordRequest(final String sourceAddress, String oldPwd, String newPwd, final IncomingObject inObj) throws SMPPException {
    final SecretChangePasswordCmd cmd = new SecretChangePasswordCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setOldPassword(oldPwd);
    cmd.setNewPassword(newPwd);

    try {
      Services.getInstance().getSecretService().execute(cmd);
      inObj.respond(Data.ESME_ROK);
      reply(inObj.getMessage(), serviceAddress, msgChangePwdOk);
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case SecretChangePasswordCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
          break;
        case SecretChangePasswordCmd.ERR_INVALID_PASSWORD:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgInvalidPassword);
          break;
        case SecretChangePasswordCmd.ERR_SYS_ERROR:
          inObj.respond(Data.ESME_RX_P_APPN);
          break;
        default:
          inObj.respond(Data.ESME_ROK);
          log.error("Unknown response code for ChPwd request: " + e.getErrCode());
      }
    }
  }

  private static void handleSecretMessage(final String sourceAddress, final String destinationAddress, String message, final IncomingObject inObj) throws SMPPException {
    final SecretSendMessageCmd cmd = new SecretSendMessageCmd();
    cmd.setSourceAddress(sourceAddress);
    cmd.setDestinationAddress(destinationAddress);
    cmd.setMessage(message);
    cmd.setDestAddressSubunit(inObj.getMessage().getDestAddrSubunit());
    cmd.setSaveDeliveryStatus(false);
    cmd.setNotifyOriginator(true);
    cmd.setSourceId(AsyncCommand.SOURCE_SMPP);

    try {
      Services.getInstance().getSecretService().execute(cmd);
      inObj.respond(Data.ESME_ROK);
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case SecretSendMessageCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED:
          inObj.respond(Data.ESME_RX_P_APPN);
          break;

        case SecretSendMessageCmd.ERR_DESTINATION_ADDRESS_IS_NOT_ALLOWED:
          inObj.respond(Data.ESME_RINVDSTADR);
          break;

        case SecretSendMessageCmd.ERR_SYS_ERROR:
          inObj.respond(Data.ESME_RX_P_APPN);
          break;

        default:
          inObj.respond(Data.ESME_ROK);
          log.error("Unknown response code for SecMsg request: " + e.getErrCode());
      }
    }
  }


  private void handleGetRequest(final String sourceAddress, String password, final IncomingObject inObj) throws SMPPException {
    final SecretGetMessagesCmd cmd = new SecretGetMessagesCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setPassword(password);

    try {
      Services.getInstance().getSecretService().execute(cmd);
      inObj.respond(Data.ESME_ROK);
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case SecretGetMessagesCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
          break;

        case SecretGetMessagesCmd.ERR_INVALID_PASSWORD:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgInvalidPassword);
          break;

        case SecretGetMessagesCmd.ERR_NO_MESSAGES:
          inObj.respond(Data.ESME_ROK);
          reply(inObj.getMessage(), serviceAddress, msgNoMessages);
          break;

        case SecretGetMessagesCmd.ERR_SYS_ERROR:
          inObj.respond(Data.ESME_RX_P_APPN);
          break;

        default:
          inObj.respond(Data.ESME_ROK);
          log.error("Unknown response code for SecMsg request: " + e.getErrCode());
      }
    }
  }
}
