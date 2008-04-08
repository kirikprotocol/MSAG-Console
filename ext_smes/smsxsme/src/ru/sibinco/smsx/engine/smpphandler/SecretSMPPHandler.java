package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.utils.config.ConfigException;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.CommandObserver;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

import java.io.File;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 29.06.2007
 */

class SecretSMPPHandler extends SMPPHandler {

  private static final Category log = Category.getInstance("SECRET SMPP");

  private final static Pattern SECRET = Pattern.compile("(S|s)(E|e|\u0415|\u0435)(C|c|\u0421|\u0441)(R|r)(E|e|\u0415|\u0435)(T|t|\u0422|\u0442)");
  private static final Pattern P = Pattern.compile("(P|p|\u0420|\u0440|\u041F|\u043F)");
  private static final Pattern SECRET_ON = Pattern.compile('(' + SECRET.pattern() + '|' + P.pattern() + ')');
  private final static Pattern NONE = Pattern.compile("(N|n)(O|o)(N|n)(E|e|\u0415|\u0435)");
  private final static Pattern SEC = Pattern.compile("(((S|s)(E|e|\u0415|\u0435)(C|c|\u0421|\u0441))|#)");
  private final static Pattern SMS = Pattern.compile("((G|g)(E|e|\u0415|\u0435)(T|t|\u0422|\u0442)|(S|s)(M|m|\u041C)(S|s))");

  private final static Pattern ONE_OR_MORE_SPACES = Pattern.compile(" +");
  private final static Pattern ANY_NONSPACE_STRING_AFTER_SPACE = Pattern.compile(ONE_OR_MORE_SPACES.pattern() + "\\S+");
  private final static Pattern ANY_STRING_AFTER_SPACE = Pattern.compile("( *| +.+)");
//  private final static String ANY_WORD = "\\s*\\S+\\s*";

  private final static Pattern SECRET_ON_REGEX = Pattern.compile(SECRET_ON.pattern()  + ANY_NONSPACE_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern SECRET_CHANGE_PASSWORD_REGEX = Pattern.compile(SECRET_ON.pattern() + ANY_NONSPACE_STRING_AFTER_SPACE.pattern() + ANY_NONSPACE_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern SECRET_OFF_REGEX = Pattern.compile('(' + SECRET.pattern() + ONE_OR_MORE_SPACES.pattern() + NONE.pattern() + "|-" + P.pattern() + ')', Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern SECRET_MESSAGE_REGEX = Pattern.compile(SEC.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern SECRET_GET_REGEX = Pattern.compile(SMS.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);


  // Properties
  private final String serviceAddress;
  private final String msgSourceAbonentNotRegistered;
  private final String msgDestinationAbonentNotRegistered;
  private final String msgSourceAbonentAlreadyRegistered;
  private final String msgUnregisterOk;
  private final String msgRegisterOk;
  private final String msgChangePwdOk;
  private final String msgInvalidPassword;
  private final String msgNoMessages;


  SecretSMPPHandler(String configDir, SMPPMultiplexor multiplexor) {
    super(multiplexor);

    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "smpphandlers/secrethandler.properties"));

      serviceAddress = config.getString("service.address");
      msgSourceAbonentNotRegistered = config.getString("source.abonent.not.registered");
      msgDestinationAbonentNotRegistered = config.getString("destination.abonent.not.registered");
      msgSourceAbonentAlreadyRegistered = config.getString("source.abonent.already.registered");
      msgUnregisterOk = config.getString("unregister.ok");
      msgRegisterOk = config.getString("register.ok");
      msgChangePwdOk = config.getString("change.password.ok");
      msgInvalidPassword = config.getString("invalid.password");
      msgNoMessages = config.getString("no.messages");
    } catch (ConfigException e) {
      throw new SMPPHandlerInitializationException(e);
    }
  }

  protected boolean handleInObj(SMPPTransportObject inObj) {
    final long start = System.currentTimeMillis();
    try {

      if (inObj.getIncomingMessage() != null && inObj.getIncomingMessage().getMessageString() != null) {
        final String msg = inObj.getIncomingMessage().getMessageString().trim();
        final String sourceAddress = inObj.getIncomingMessage().getSourceAddress();
        final String destinationAddress = inObj.getIncomingMessage().getDestinationAddress();

        log.info("Msg srcaddr=" + sourceAddress + "; dstaddr=" + destinationAddress);

        if (destinationAddress.equalsIgnoreCase(serviceAddress)) {

          if (SECRET_OFF_REGEX.matcher(msg).matches()) { // Unsubscription request
            handleOffRequest(sourceAddress, inObj.getIncomingMessage());
            return true;

          } else if (SECRET_CHANGE_PASSWORD_REGEX.matcher(msg).matches()) { // Change password request
            // Separate old and new passwords
            final String passwords = getPasswordInSecret(msg).trim();
            int pos = passwords.indexOf(" ");
            final String oldPassword = passwords.substring(0, pos).trim();
            final String newPassword = passwords.substring(pos).trim();

            handleChangePasswordRequest(sourceAddress, oldPassword, newPassword, inObj.getIncomingMessage());
            return true;

          } else if (SECRET_ON_REGEX.matcher(msg).matches()) { // Subscription request
            handleOnRequest(sourceAddress, getPasswordInSecret(msg).trim(), inObj.getIncomingMessage());
            return true;

          } else if (SECRET_GET_REGEX.matcher(msg).matches()) { // Get messages request
            handleGetRequest(sourceAddress, getPasswordInGet(msg).trim(), inObj.getIncomingMessage());
            return true;
          }

        } else if (SECRET_MESSAGE_REGEX.matcher(msg).matches()) { // Send secret message request
          handleSecretMessage(sourceAddress, destinationAddress, getMessage(msg), inObj.getIncomingMessage());
          return true;
        }

        log.info("Msg format is unknown");
      }

      return false;

    } catch (Throwable e) {
      log.error(e,e);
      sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
      return true;

    } finally {
      log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  private void handleOffRequest(final String sourceAddress, final Message msg) {
    final SecretUnregisterAbonentCmd cmd = new SecretUnregisterAbonentCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretUnregisterAbonentCmd cmd = (SecretUnregisterAbonentCmd)command;
        switch (cmd.getStatus()) {
          case SecretUnregisterAbonentCmd.STATUS_SUCCESS:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgUnregisterOk);
            break;
          case SecretUnregisterAbonentCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
            break;
          case SecretUnregisterAbonentCmd.STATUS_SYSTEM_ERROR:
            sendResponse(msg, Data.ESME_RSYSERR);
            break;
          default:
            sendResponse(msg, Data.ESME_ROK);
            log.error("Unknown response code for Off request: " + cmd.getStatus());
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }



  private void handleOnRequest(final String sourceAddress, String password, final Message msg) {
    final SecretRegisterAbonentCmd cmd = new SecretRegisterAbonentCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setPassword(password);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretRegisterAbonentCmd cmd = (SecretRegisterAbonentCmd)command;
        switch (cmd.getStatus()) {
          case SecretRegisterAbonentCmd.STATUS_SUCCESS:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgRegisterOk);
            break;
          case SecretRegisterAbonentCmd.STATUS_SOURCE_ABONENT_ALREADY_REGISTERED:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgSourceAbonentAlreadyRegistered.replaceAll("\\{abonent}", sourceAddress));
            break;
          case SecretRegisterAbonentCmd.STATUS_SYSTEM_ERROR:
            sendResponse(msg, Data.ESME_RSYSERR);
            break;
          default:
            sendResponse(msg, Data.ESME_ROK);
            log.error("Unknown response code for On request: " + cmd.getStatus());
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }

  private void handleChangePasswordRequest(final String sourceAddress, String oldPwd, String newPwd, final Message msg) {
    final SecretChangePasswordCmd cmd = new SecretChangePasswordCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setOldPassword(oldPwd);
    cmd.setNewPassword(newPwd);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretChangePasswordCmd cmd = (SecretChangePasswordCmd)command;
        switch (cmd.getStatus()) {
          case SecretChangePasswordCmd.STATUS_SUCCESS:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgChangePwdOk);
            break;
          case SecretChangePasswordCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
            break;
          case SecretChangePasswordCmd.STATUS_INVALID_PASSWORD:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgInvalidPassword);
            break;
          case SecretChangePasswordCmd.STATUS_SYSTEM_ERROR:
            sendResponse(msg, Data.ESME_RSYSERR);
            break;
          default:
            sendResponse(msg, Data.ESME_ROK);
            log.error("Unknown response code for ChPwd request: " + cmd.getStatus());
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }

  private void handleSecretMessage(final String sourceAddress, final String destinationAddress, String message, final Message msg) {
    final SecretSendMessageCmd cmd = new SecretSendMessageCmd();
    cmd.setSourceAddress(sourceAddress);
    cmd.setDestinationAddress(destinationAddress);
    cmd.setMessage(message);
    cmd.setDestAddressSubunit(msg.getDestAddrSubunit());
    cmd.setSaveDeliveryStatus(false);
    cmd.setNotifyOriginator(true);
    cmd.setSourceId(Command.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver(){
      public void update(Command command) {
        final SecretSendMessageCmd cmd = (SecretSendMessageCmd)command;
        switch (cmd.getStatus()) {
          case SecretSendMessageCmd.STATUS_SUCCESS:
            sendResponse(msg, Data.ESME_ROK);
            break;

          case SecretSendMessageCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
            sendResponse(msg, Data.ESME_RX_P_APPN);
            break;

          case SecretSendMessageCmd.STATUS_DESTINATION_ABONENT_NOT_REGISTERED:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgDestinationAbonentNotRegistered.replaceAll("\\{dest_abonent}", destinationAddress));
            break;

          case SecretSendMessageCmd.STATUS_DESTINATION_ADDRESS_IS_NOT_ALLOWED:
            sendResponse(msg, Data.ESME_RINVDSTADR);
            break;

          case SecretSendMessageCmd.STATUS_SYSTEM_ERROR:
            sendResponse(msg, Data.ESME_RSYSERR);
            break;

          default:
            sendResponse(msg, Data.ESME_ROK);
            log.error("Unknown response code for SecMsg request: " + cmd.getStatus());
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }


  private void handleGetRequest(final String sourceAddress, String password, final Message msg) {
    final SecretGetMessagesCmd cmd = new SecretGetMessagesCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setPassword(password);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final SecretGetMessagesCmd cmd = (SecretGetMessagesCmd)command;
        switch (cmd.getStatus()) {
          case SecretGetMessagesCmd.STATUS_SUCCESS:
            sendResponse(msg, Data.ESME_ROK);
            break;

          case SecretGetMessagesCmd.STATUS_SOURCE_ABONENT_NOT_REGISTERED:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered.replaceAll("\\{abonent}", sourceAddress));
            break;

          case SecretGetMessagesCmd.STATUS_INVALID_PASSWORD:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgInvalidPassword);
            break;

          case SecretGetMessagesCmd.STATUS_NO_MESSAGES:
            sendResponse(msg, Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgNoMessages);
            break;

          case SecretGetMessagesCmd.STATUS_SYSTEM_ERROR:
            sendResponse(msg, Data.ESME_RSYSERR);
            break;

          default:
            sendResponse(msg, Data.ESME_ROK);
            log.error("Unknown response code for SecMsg request: " + cmd.getStatus());
        }
      }
    });
    ServiceManager.getInstance().getSecretService().execute(cmd);
  }

  private static String removePrefix(final String message, final Pattern prefixPattern) {
    final Matcher matcher = prefixPattern.matcher(message);
    matcher.find();
    return message.substring(matcher.end());
  }

  private static String getMessage(final String message) {
    return removePrefix(message, SEC);
  }

  private static String getPasswordInSecret(final String message) {
    return removePrefix(message, SECRET_ON);
  }

  private static String getPasswordInGet(final String message) {
    return removePrefix(message, SMS);
  }

  public static void main(String[] args) {
    System.out.println(SECRET_MESSAGE_REGEX.matcher("SEC lfgskldfglksdfg").matches());
  }

}
