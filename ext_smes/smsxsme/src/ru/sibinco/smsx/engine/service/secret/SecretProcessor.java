package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.utils.ds.DataSourceException;
import com.eyeline.sme.utils.encode.MessageEncoder;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretUser;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretUserWithMessages;
import ru.sibinco.smsx.engine.service.Command;

import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 29.06.2007
 */

class SecretProcessor implements SecretChangePasswordCmd.Receiver, SecretGetMessagesCmd.Receiver,
                                 SecretGetMessageStatusCmd.Receiver, SecretRegisterAbonentCmd.Receiver,
                                 SecretSendMessageCmd.Receiver, SecretUnregisterAbonentCmd.Receiver {

  private static final Category log = Category.getInstance("SECRET");

  private static final Pattern ALLOWED_DEST_ADDR = Pattern.compile("\\+\\d{11}");

  private final SecretDataSource ds;
  private final MessageSender messageSender;

  SecretProcessor(SecretDataSource ds, MessageSender messageSender) {
    this.ds = ds;
    this.messageSender = messageSender;
  }


  public void execute(SecretRegisterAbonentCmd cmd) {
    int status;
    try {
      if (log.isInfoEnabled())
        log.info("ON req: srcaddr=" + cmd.getAbonentAddress());

      // Check abonent's registration
      if (ds.loadSecretUserByAddress(cmd.getAbonentAddress()) != null) { // Abonent has been registered
        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " has already registered in DB");
        status = SecretRegisterAbonentCmd.STATUS_SOURCE_ABONENT_ALREADY_REGISTERED;

      } else { // Abonent has not been registered
        ds.saveSecretUser(new SecretUser(cmd.getAbonentAddress(), MessageEncoder.encodeMD5(cmd.getPassword())));
        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " registered in DB");
        status = SecretRegisterAbonentCmd.STATUS_SUCCESS;
      }

    } catch (Throwable e) {
      log.error("ON error for " + cmd.getAbonentAddress(), e);
      status = SecretRegisterAbonentCmd.STATUS_SYSTEM_ERROR;
    }

    cmd.update(status);
  }


  public void execute(SecretUnregisterAbonentCmd cmd) {
    int status;

    try {
      if (log.isInfoEnabled())
        log.info("OFF req: srcaddr=" + cmd.getAbonentAddress());

      // Check abonent's registration
      final SecretUser secretUser = ds.loadSecretUserByAddress(cmd.getAbonentAddress());
      if (secretUser != null) { // Abonent has been registered in DB

        // Remove messages
        final Collection messages = ds.loadSecretMessagesByAddress(cmd.getAbonentAddress());
        for (Iterator iter = messages.iterator(); iter.hasNext();)
          ds.removeSecretMessage((SecretMessage) iter.next());

        // Remove user
        ds.removeSecretUser(secretUser);

        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " and all his messages removed from DB");
        status = SecretUnregisterAbonentCmd.STATUS_SUCCESS;

      } else { // Abonent has not been registered

        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " not registered in DB");
        status = SecretUnregisterAbonentCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED;
      }

    } catch (Throwable e) {
      log.error("OFF error for " + cmd.getAbonentAddress(), e);
      status = SecretUnregisterAbonentCmd.STATUS_SYSTEM_ERROR;

    }

    cmd.update(status);
  }

  public void execute(SecretChangePasswordCmd cmd) {
    int status;

    try {
      if (log.isInfoEnabled())
        log.info("ChPwd req: srcaddr=" + cmd.getAbonentAddress());

      final SecretUser user = ds.loadSecretUserByAddress(cmd.getAbonentAddress());

      if (user == null) { // Abonent has not been registered
        log.info("Src abonent " + cmd.getAbonentAddress() + " not registered in DB");
        status = SecretChangePasswordCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED;

      } else if (!user.getPassword().equals(MessageEncoder.encodeMD5(cmd.getOldPassword()))) {  // Check old password
        if (log.isInfoEnabled())
          log.info("Old pwd is invalid");
        status = SecretChangePasswordCmd.STATUS_INVALID_PASSWORD;

      } else {

        // Update password
        user.setPassword(MessageEncoder.encodeMD5(cmd.getNewPassword()));
        ds.saveSecretUser(user);
        if (log.isInfoEnabled())
          log.info("Pwd was updated for " + cmd.getAbonentAddress());

        status = SecretChangePasswordCmd.STATUS_SUCCESS;
      }
    } catch (Throwable e) {
      log.error("ChPwd error for " + cmd.getAbonentAddress(),e);
      status = SecretChangePasswordCmd.STATUS_SYSTEM_ERROR;

    }

    cmd.update(status);
  }

  public void execute(SecretSendMessageCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Sec msg send req: srcaddr=" + cmd.getSourceAddress() + "; dstAddr=" + cmd.getDestinationAddress());

      // Check destination address
      if (!ALLOWED_DEST_ADDR.matcher(cmd.getDestinationAddress()).matches()) {
        if (log.isInfoEnabled())
          log.info("Dst addr is not allowed");
        cmd.update(SecretSendMessageCmd.STATUS_DESTINATION_ADDRESS_IS_NOT_ALLOWED);
        return;
      }

      final Map users = ds.loadSecretUsersByAddresses(new String[]{cmd.getSourceAddress(), cmd.getDestinationAddress()});

      if (cmd.isNotifyOriginator() && users.get(cmd.getSourceAddress()) == null) { // sourceAddress has not been registered
        if (log.isInfoEnabled())
          log.info("Src abonent " + cmd.getSourceAddress() + " not registered in DB.");
        cmd.update(SecretSendMessageCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED);
        return;
      }

      if (users.get(cmd.getDestinationAddress()) == null) { // destinationAddress has not been registered
        if (log.isInfoEnabled())
          log.info("Dst abonent " + cmd.getDestinationAddress() + " not registered in DB");
        messageSender.sendInvitationMessage(cmd.getDestinationAddress());
        cmd.update(SecretSendMessageCmd.STATUS_DESTINATION_ABONENT_NOT_REGISTERED);
        return;
      }

      // Store message
      final SecretMessage secretMessage = new SecretMessage();
      secretMessage.setDestinationAddress(cmd.getDestinationAddress());
      secretMessage.setMessage(cmd.getMessage());
      secretMessage.setSourceAddress(cmd.getSourceAddress());
      secretMessage.setDestAddressSubunit(cmd.getDestAddressSubunit());
      secretMessage.setSaveDeliveryStatus(cmd.isSaveDeliveryStatus());
      secretMessage.setNotifyOriginator(cmd.isNotifyOriginator());
      secretMessage.setConnectionName(cmd.getSourceId() == Command.SOURCE_SMPP ? "smsx" : "websms");
      ds.saveSecretMessage(secretMessage);

      messageSender.sendInformMessage(secretMessage);
      cmd.setMsgId(secretMessage.getId());
      cmd.update(SecretSendMessageCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error("Secret msg send error for " + cmd.getSourceAddress(), e);
      cmd.update(SecretSendMessageCmd.STATUS_SYSTEM_ERROR);
    }
  }


  public void execute(SecretGetMessagesCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Msgs get req: srcaddr=" + cmd.getAbonentAddress());

      final SecretUserWithMessages userWithMessages = ds.loadSecretUserWithMessages(cmd.getAbonentAddress());

      // Check user's registration
      if (userWithMessages == null) {
        if (log.isInfoEnabled())
          log.info("Src abonent " + cmd.getAbonentAddress() + " not registered in DB");
        cmd.update(SecretGetMessagesCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED);
        return;
      }

      // Check user's password
      if (!userWithMessages.getUser().getPassword().equals(MessageEncoder.encodeMD5(cmd.getPassword()))) {
        if (log.isInfoEnabled())
          log.info("Pwd is wrong for " + cmd.getAbonentAddress());
        cmd.update(SecretGetMessagesCmd.STATUS_INVALID_PASSWORD);
        return;
      }

      final Collection messages = userWithMessages.getMessages();

      // Load and send messages for abonentAddress
      if (messages.isEmpty()) { // No messages has been found
        if (log.isInfoEnabled())
          log.info("No msgs were found for " + cmd.getAbonentAddress());
        cmd.update(SecretGetMessagesCmd.STATUS_NO_MESSAGES);
        return;

      } else { // Some messages has been found
        if (log.isInfoEnabled())
          log.info(messages.size() + " messages were found for abonent " + cmd.getAbonentAddress());
        for (Iterator iter = messages.iterator(); iter.hasNext();)
          messageSender.sendSecretMessage((SecretMessage) iter.next());

        if (log.isInfoEnabled())
          log.info("Msgs get req processed");
        cmd.update(SecretGetMessagesCmd.STATUS_SUCCESS);
      }
    } catch (Exception e) {
      log.error("Msgs get req error for " + cmd.getAbonentAddress(), e);
      cmd.update(SecretGetMessagesCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(SecretGetMessageStatusCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Get msg status: id=" + cmd.getMsgId());

      final SecretMessage msg = ds.loadSecretMessageById(cmd.getMsgId());

      if (msg != null && msg.isSaveDeliveryStatus()) {
        cmd.setMessageStatus(msg.getStatus());
        cmd.setSmppStatus(msg.getSmppStatus());

      } else {
        if (log.isInfoEnabled())
          log.info("Msg with id=" + cmd.getMsgId() + " not found");
        cmd.setMessageStatus(SecretGetMessageStatusCmd.MESSAGE_STATUS_UNKNOWN);
      }

      cmd.update(SecretGetMessageStatusCmd.STATUS_SUCCESS);

    } catch (DataSourceException e) {
      cmd.update(SecretGetMessageStatusCmd.STATUS_SYSTEM_ERROR);
    }
  }

}
