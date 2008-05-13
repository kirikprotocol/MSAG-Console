package ru.sibinco.smsx.engine.smpphandler;

import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.engine.service.nick.commands.NickRegisterCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickSendMessageCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickUnregisterCmd;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.CommandObserver;
import ru.sibinco.smsx.engine.service.Command;
import ru.aurorisoft.smpp.Message;
import com.logica.smpp.Data;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import org.apache.log4j.Category;

import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.File;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

class NickSMPPHandler extends SMPPHandler {

  private static final Category log = Category.getInstance("NICK SMPP");

  private static final Pattern NICK = Pattern.compile("(N|n)(I|i)(C|c)(K|k)");
  private static final Pattern SET_NICK = Pattern.compile("(S|s)(E|e)(T|t)(N|n)(I|i)(C|c)(K|k)");
  private static final Pattern CLEAR_NICK = Pattern.compile("(C|c)(L|l)(E|e)(A|a)(R|r)(N|n)(I|i)(C|c)(K|k)");
  private static final Pattern ANY_NONSPACE_STRING_AFTER_SPACE = Pattern.compile("\\s+\\S+");
  private static final Pattern ADDRESS = Pattern.compile("\\+{0,1}\\d{1,11}");

  private static final Pattern REGISTER_NICK = Pattern.compile(SET_NICK.pattern() + ANY_NONSPACE_STRING_AFTER_SPACE.pattern() + "\\s*");
  private static final Pattern UNREGISTER_NICK = Pattern.compile(CLEAR_NICK.pattern() + "\\s*");
  private static final Pattern SEND_NICK = Pattern.compile(NICK.pattern() + "\\s+" + ADDRESS.pattern() + "\\s*" + ".*");

  private final String serviceAddress;

  public NickSMPPHandler(String configDir, SMPPMultiplexor multiplexor) {
    super(multiplexor);

    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "smpphandlers/nickhandler.properties"));
      serviceAddress = config.getString("service.address");
    } catch (Throwable e) {
      throw new SMPPHandlerInitializationException(e);
    }
  }

  protected boolean handleInObj(SMPPTransportObject inObj) {
    final long start = System.currentTimeMillis();
    try {
      final String msg = inObj.getIncomingMessage().getMessageString().trim();
      final String sourceAddress = inObj.getIncomingMessage().getSourceAddress();
      final String destinationAddress = inObj.getIncomingMessage().getDestinationAddress();

      if (log.isInfoEnabled())
        log.info("Msg srcaddr=" + sourceAddress + "; dstaddr=" + destinationAddress);

      if (SEND_NICK.matcher(msg).matches() && destinationAddress.equalsIgnoreCase(serviceAddress)) {
        final String nickAndMessage = removePrefix(msg, NICK).trim();
        int pos = nickAndMessage.indexOf(" ");
        final String message = (pos > 0) ? nickAndMessage.substring(pos).trim() : "";
        sendNickMessage(sourceAddress, destinationAddress, message, inObj.getIncomingMessage());
        return true;

      } else if (REGISTER_NICK.matcher(msg).matches() && destinationAddress.equalsIgnoreCase(serviceAddress)) {
        registerNick(sourceAddress, removePrefix(msg, SET_NICK).trim(), inObj.getIncomingMessage());
        return true;

      } else if (UNREGISTER_NICK.matcher(msg).matches() && destinationAddress.equalsIgnoreCase(serviceAddress)) {
        unregisterNick(sourceAddress, inObj.getIncomingMessage());
        return true;
      }

      if (log.isInfoEnabled())
        log.info("Msg format is unknown");
      return false;

    } catch (Throwable e) {
      log.error(e,e);
      sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
      return true;

    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }

  }

  private void unregisterNick(String sourceAddress, final Message message) {
    final NickUnregisterCmd cmd = new NickUnregisterCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setSourceId(Command.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final NickUnregisterCmd cmd = (NickUnregisterCmd)command;
        
        switch (cmd.getStatus()) {
          case NickUnregisterCmd.STATUS_DELIVERED:
            sendResponse(message, cmd.getSmppStatus());
            break;
          case NickUnregisterCmd.STATUS_SYSTEM_ERROR:
            sendResponse(message, Data.ESME_RSYSERR);
        }
      }
    });
    ServiceManager.getInstance().getNickService().execute(cmd);
  }

  private void registerNick(String sourceAddress, String nick, final Message message) {
    final NickRegisterCmd cmd = new NickRegisterCmd();
    cmd.setAbonentAddress(sourceAddress);
    cmd.setNick(nick);
    cmd.setSourceId(Command.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final NickRegisterCmd cmd = (NickRegisterCmd)command;

        switch (cmd.getStatus()) {
          case NickRegisterCmd.STATUS_INVALID_ABONENT_ADDRESS:
          case NickRegisterCmd.STATUS_INVALID_NICK:
            log.error("Invalid nick or abonent address");
            sendResponse(message, Data.ESME_RSYSERR);
            break;
          case NickRegisterCmd.STATUS_DELIVERED:
            sendResponse(message, cmd.getSmppStatus());
            break;
          case NickRegisterCmd.STATUS_SYSTEM_ERROR:
            sendResponse(message, Data.ESME_RSYSERR);
        }
      }
    });
    ServiceManager.getInstance().getNickService().execute(cmd);
  }

  private void sendNickMessage(String sourceAddress, String destinationAddress, String message, final Message msg) {
    final NickSendMessageCmd cmd = new NickSendMessageCmd();
    cmd.setSourceAddress(sourceAddress);
    cmd.setDestinationAddress(destinationAddress);
    cmd.setMessage(message);
    cmd.setMscAddress(msg.getMscAddress());
    cmd.setImsi(msg.getImsi());
    cmd.setSourceId(Command.SOURCE_SMPP);
    cmd.addExecutionObserver(new CommandObserver() {
      public void update(Command command) {
        final NickSendMessageCmd cmd = (NickSendMessageCmd)command;

        switch (cmd.getStatus()) {
          case NickSendMessageCmd.STATUS_INVALID_DESTINATION_ADDRESS:
          case NickSendMessageCmd.STATUS_INVALID_SOURCE_ADDRESS:
            log.error("Invalid src or dst addr");
            sendResponse(msg, Data.ESME_RSYSERR);
            break;
          case NickSendMessageCmd.STATUS_DELIVERED:
            sendResponse(msg, cmd.getSmppStatus());
            break;
          case NickSendMessageCmd.STATUS_SYSTEM_ERROR:
            sendResponse(msg, Data.ESME_RSYSERR);
        }
      }
    });
    ServiceManager.getInstance().getNickService().execute(cmd);
  }

  private static String removePrefix(final String message, final Pattern prefixPattern) {
    final Matcher matcher = prefixPattern.matcher(message);
    matcher.find();
    return message.substring(matcher.end());
  }

  public static void main(String[] args) {
    final String str = "nick +79607891901 hehe";
    final String addr = "+79607891901";

    System.out.println(SEND_NICK.matcher(str).matches());
    System.out.println(ADDRESS.matcher(addr).matches());
  }
}
