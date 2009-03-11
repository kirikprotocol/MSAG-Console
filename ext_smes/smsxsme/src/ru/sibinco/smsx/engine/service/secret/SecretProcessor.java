package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.utils.encode.MessageEncoder;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretUser;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretUserWithMessages;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.*;
import java.util.Collection;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 29.06.2007
 */

class SecretProcessor implements SecretChangePasswordCmd.Receiver, SecretGetMessagesCmd.Receiver,
                                 SecretGetMessageStatusCmd.Receiver, SecretRegisterAbonentCmd.Receiver,
                                 SecretSendMessageCmd.Receiver, SecretUnregisterAbonentCmd.Receiver,
                                 SecretHandleReceiptCmd.Receiver {

  private static final Category log = Category.getInstance("SECRET");

  private static final Pattern ALLOWED_DEST_ADDR = Pattern.compile("\\+\\d{11}|0012");

  private final SecretDataSource ds;
  private final MessageSender messageSender;

  SecretProcessor(SecretDataSource ds, MessageSender messageSender) {
    this.ds = ds;
    this.messageSender = messageSender;
  }


  public void execute(SecretRegisterAbonentCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("ON req: srcaddr=" + cmd.getAbonentAddress());

      // Check abonent's registration
      if (ds.loadSecretUserByAddress(cmd.getAbonentAddress()) != null) { // Abonent has been registered
        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " has already registered in DB");
        throw new CommandExecutionException("Abonent "  + cmd.getAbonentAddress() + " has already registered in DB", SecretRegisterAbonentCmd.ERR_SOURCE_ABONENT_ALREADY_REGISTERED);

      } else { // Abonent has not been registered
        ds.saveSecretUser(new SecretUser(cmd.getAbonentAddress(), MessageEncoder.encodeMD5(cmd.getPassword())));
        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " registered in DB");
      }

    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("ON error for " + cmd.getAbonentAddress(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretRegisterAbonentCmd.ERR_SYS_ERROR);
    }
  }


  public void execute(SecretUnregisterAbonentCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("OFF req: srcaddr=" + cmd.getAbonentAddress());

      // Check abonent's registration
      final SecretUser secretUser = ds.loadSecretUserByAddress(cmd.getAbonentAddress());
      if (secretUser != null) { // Abonent has been registered in DB

        // Remove messages
        final Collection<SecretMessage> messages = ds.loadSecretMessagesByAddress(cmd.getAbonentAddress());
        for (SecretMessage message : messages)
          ds.removeSecretMessage(message);

        // Remove user
        ds.removeSecretUser(secretUser);

        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " and all his messages removed from DB");

      } else { // Abonent has not been registered

        if (log.isInfoEnabled())
          log.info("Abonent " + cmd.getAbonentAddress() + " not registered in DB");
        throw new CommandExecutionException("Abonent " + cmd.getAbonentAddress() + " not registered in DB", SecretUnregisterAbonentCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED);
      }

    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("OFF error for " + cmd.getAbonentAddress(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretUnregisterAbonentCmd.ERR_SYS_ERROR);
    }
  }

  public void execute(SecretChangePasswordCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("ChPwd req: srcaddr=" + cmd.getAbonentAddress());

      final SecretUser user = ds.loadSecretUserByAddress(cmd.getAbonentAddress());

      if (user == null) { // Abonent has not been registered
        log.info("Src abonent " + cmd.getAbonentAddress() + " not registered in DB");
        throw new CommandExecutionException("Src abonent " + cmd.getAbonentAddress() + " not registered in DB", SecretChangePasswordCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED);
      }

      if (!user.getPassword().equals(MessageEncoder.encodeMD5(cmd.getOldPassword()))) {  // Check old password
        if (log.isInfoEnabled())
          log.info("Old pwd is invalid");
        throw new CommandExecutionException("Old pwd is invalid", SecretChangePasswordCmd.ERR_INVALID_PASSWORD);
      }

      // Update password
      user.setPassword(MessageEncoder.encodeMD5(cmd.getNewPassword()));
      ds.saveSecretUser(user);
      if (log.isInfoEnabled())
        log.info("Pwd was updated for " + cmd.getAbonentAddress());

    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("ChPwd error for " + cmd.getAbonentAddress(),e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretChangePasswordCmd.ERR_SYS_ERROR);
    }
  }

  public long execute(SecretSendMessageCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Sec msg send req: srcaddr=" + cmd.getSourceAddress() + "; dstAddr=" + cmd.getDestinationAddress());

      // Check destination address
      if (!ALLOWED_DEST_ADDR.matcher(cmd.getDestinationAddress()).matches()) {
        if (log.isInfoEnabled())
          log.info("Dst addr is not allowed");
        throw new CommandExecutionException("Dst addr id not allowed", SecretSendMessageCmd.ERR_DESTINATION_ADDRESS_IS_NOT_ALLOWED);
      }

      final Map<String, SecretUser> users = ds.loadSecretUsersByAddresses(new String[]{cmd.getSourceAddress(), cmd.getDestinationAddress()});

      if (cmd.isNotifyOriginator() && users.get(cmd.getSourceAddress()) == null) { // sourceAddress has not been registered
        if (log.isInfoEnabled())
          log.info("Src abonent " + cmd.getSourceAddress() + " not registered in DB.");
        throw new CommandExecutionException("Src abonent " + cmd.getSourceAddress() + " not registered in DB.", SecretSendMessageCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED);
      }

      // Store message
      final SecretMessage secretMessage = new SecretMessage();
      secretMessage.setDestinationAddress(cmd.getDestinationAddress());
      secretMessage.setMessage(cmd.getMessage());
      secretMessage.setSourceAddress(cmd.getSourceAddress());
      secretMessage.setDestAddressSubunit(cmd.getDestAddressSubunit());
      secretMessage.setSaveDeliveryStatus(cmd.isSaveDeliveryStatus());
      secretMessage.setNotifyOriginator(cmd.isNotifyOriginator());
      secretMessage.setConnectionName(cmd.getSourceId() == AsyncCommand.SOURCE_SMPP ? "smsx" : "websms");
      secretMessage.setMscAddress(cmd.getMscAddress());
      secretMessage.setAppendAdvertising(cmd.isAppendAdverising());
      ds.saveSecretMessage(secretMessage);

      if (users.get(cmd.getDestinationAddress()) == null) // destinationAddress has not been registered
        messageSender.sendInvitationMessage(cmd.getDestinationAddress());
      else
        messageSender.sendInformMessage(secretMessage);

      return secretMessage.getId();

    } catch (CommandExecutionException e) {
      throw e;
    } catch (Throwable e) {
      log.error("Secret msg send error for " + cmd.getSourceAddress(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretSendMessageCmd.ERR_SYS_ERROR);
    }
  }


  public void execute(SecretGetMessagesCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Msgs get req: srcaddr=" + cmd.getAbonentAddress());

      final SecretUserWithMessages userWithMessages = ds.loadSecretUserWithMessages(cmd.getAbonentAddress());

      // Check user's registration
      if (userWithMessages == null) {
        if (log.isInfoEnabled())
          log.info("Src abonent " + cmd.getAbonentAddress() + " not registered in DB");
        throw new CommandExecutionException("Src abonent " + cmd.getAbonentAddress() + " not registered in DB", SecretGetMessagesCmd.ERR_SOURCE_ABONENT_NOT_REGISTERED);
      }

      // Check user's password
      if (!userWithMessages.getUser().getPassword().equals(MessageEncoder.encodeMD5(cmd.getPassword()))) {
        if (log.isInfoEnabled())
          log.info("Pwd is wrong for " + cmd.getAbonentAddress());
        throw new CommandExecutionException("Pwd is wrong for " + cmd.getAbonentAddress(), SecretGetMessagesCmd.ERR_INVALID_PASSWORD);
      }

      final Collection messages = userWithMessages.getMessages();

      // Load and send messages for abonentAddress
      if (messages.isEmpty()) { // No messages has been found
        if (log.isInfoEnabled())
          log.info("No msgs were found for " + cmd.getAbonentAddress());
        throw new CommandExecutionException("No msgs were found for " + cmd.getAbonentAddress(), SecretGetMessagesCmd.ERR_NO_MESSAGES);
      }

      // Some messages has been found
      if (log.isInfoEnabled())
        log.info(messages.size() + " messages were found for abonent " + cmd.getAbonentAddress());
      for (Object message : messages) messageSender.sendSecretMessage((SecretMessage) message);

      if (log.isInfoEnabled())
        log.info("Msgs get req processed");

    } catch (CommandExecutionException e) {
      throw e;
    } catch (Exception e) {
      log.error("Msgs get req error for " + cmd.getAbonentAddress(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretGetMessagesCmd.ERR_SYS_ERROR);
    }
  }

  public int execute(SecretGetMessageStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Get msg status: id=" + cmd.getMsgId());

      final SecretMessage msg = ds.loadSecretMessageById(cmd.getMsgId());

      if (msg != null && msg.isSaveDeliveryStatus()) {
        return msg.getStatus();
      } else {
        if (log.isInfoEnabled())
          log.info("Msg with id=" + cmd.getMsgId() + " not found");
        return SecretGetMessageStatusCmd.MESSAGE_STATUS_UNKNOWN;
      }

    } catch (DataSourceException e) {
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretGetMessageStatusCmd.ERR_SYS_ERROR);
    }
  }

  public boolean execute(SecretHandleReceiptCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Handle rcpt: id=" + cmd.getSmppMessageId()    + "; dlvr=" + cmd.isDelivered());

      int result = ds.updateMessageStatus(cmd.getSmppMessageId(), cmd.isDelivered() ? SecretMessage.STATUS_DELIVERED : SecretMessage.STATUS_DELIVERY_FAILED);
      return result > 0;

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SecretHandleReceiptCmd.ERR_SYS_ERROR);
    }
  }
}
