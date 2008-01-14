package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.utils.config.ConfigException;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.CommandObserver;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.sponsored.commands.SponsoredRegisterAbonentCmd;
import ru.sibinco.smsx.engine.service.sponsored.commands.SponsoredUnregisterAbonentCmd;
import ru.sibinco.smsx.engine.service.sponsored.commands.SponsoredRegisterDeliveryCmd;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;
import ru.aurorisoft.smpp.Message;

import java.io.File;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 02.07.2007
 */

class SponsoredSMPPHandler extends SMPPHandler {

  private static final Category log = Category.getInstance("SPONSORED SMPP");

  private static final Pattern SPONSORED = Pattern.compile("(S|s)(P|p)(O|o)(N|n)(S|s)(O|o)(R|r)(E|e)(D|d)");
  private static final Pattern SPONSOR = Pattern.compile("(S|s)(P|p)(O|o)(N|n)(S|s)(O|o)(R|r)\\s*");
  private static final Pattern NONE = Pattern.compile("(N|n)(O|o)(N|n)(E|e)");
  private static final Pattern ANY_NUMBER = Pattern.compile("\\d+");
  private static final Pattern SPACE = Pattern.compile(" ");
  private static final Pattern ONE_OR_MORE_SPACES = Pattern.compile(" +");
  private static final SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy");

  private static final Pattern SPONSORED_SUBSCRIBE = Pattern.compile(SPONSORED.pattern() + ONE_OR_MORE_SPACES.pattern() + ANY_NUMBER.pattern());
  private static final Pattern SPONSORED_UNSUBSCRIBE = Pattern.compile(SPONSORED.pattern() + ONE_OR_MORE_SPACES.pattern() + NONE.pattern());

  private final String serviceAddress;
  private final String msgSubscriptionOkNow;
  private final String msgSubscriptionOkTomorrow;
  private final String msgWrongSubscriptionCount;
  private final String msgUnsubscriptionOk;
  private final String msgSourceAbonentNotRegistered;
  private final String msgSubscriptionLocked;
  private final String msgInvitation;

  SponsoredSMPPHandler(String configDir, SMPPMultiplexor multiplexor) {
    super(multiplexor);

    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "smpphandlers/sponsoredhandler.properties"));
      serviceAddress = config.getString("service.address");
      msgSubscriptionOkNow = config.getString("subscription.ok");
      msgSubscriptionOkTomorrow = config.getString("subscription.ok.tomorrow");
      msgUnsubscriptionOk = config.getString("unsubscription.ok");
      msgWrongSubscriptionCount = config.getString("wrong.subscription.count");
      msgSourceAbonentNotRegistered = config.getString("source.abonent.not.registered");
      msgSubscriptionLocked = config.getString("subscription.locked");
      msgInvitation = config.getString("invitation");
    } catch (ConfigException e) {
      throw new SMPPHandlerInitializationException(e);
    }
  }

  protected boolean handleInObj(final SMPPTransportObject inObj) {
    final long start = System.currentTimeMillis();

    try {
      if (inObj.getIncomingMessage() != null && inObj.getIncomingMessage().getMessageString() != null) {
        final String msg = inObj.getIncomingMessage().getMessageString().trim();
        final String sourceAddress = inObj.getIncomingMessage().getSourceAddress();
        final String destinationAddress = inObj.getIncomingMessage().getDestinationAddress();

        log.info("Msg srcaddr=" + sourceAddress + "; dstaddr=" + inObj.getIncomingMessage().getDestinationAddress());

        if (destinationAddress.equalsIgnoreCase(serviceAddress)) {

          if (inObj.getIncomingMessage().isReceipt()) {
            sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
            handleDeliveryReport(inObj.getIncomingMessage());
            return true;
          }

          if (SPONSOR.matcher(msg).matches()) {
            sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgInvitation);
            return true;

          } else if (SPONSORED_SUBSCRIBE.matcher(msg).matches()) {
            try {
              final int count = Integer.parseInt(SPACE.split(msg)[1]);
              final SponsoredRegisterAbonentCmd cmd = new SponsoredRegisterAbonentCmd();
              cmd.setAbonentAddress(sourceAddress);
              cmd.setCount(count);
              cmd.setSourceId(Command.SOURCE_SMPP);
              cmd.addExecutionObserver(new CommandObserver(){
                public void update(Command command) {
                  final SponsoredRegisterAbonentCmd cmd = (SponsoredRegisterAbonentCmd)command;
                  switch (cmd.getStatus()) {
                    case SponsoredRegisterAbonentCmd.STATUS_SUCCESS:
                      sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                      final MessageFormat mf = new MessageFormat(cmd.isAbonentExists() ? msgSubscriptionOkTomorrow : msgSubscriptionOkNow);
                      final String response = mf.format(new Object[]{new Integer(count), dateFormat.format(cmd.getProfileChangeDate())});
                      sendMessage(serviceAddress, sourceAddress, response);
                      break;
                    case SponsoredRegisterAbonentCmd.STATUS_INVALID_SUBSCRIPTION_COUNT:
                      sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                      sendMessage(serviceAddress, sourceAddress, msgWrongSubscriptionCount);
                      break;
                    case SponsoredRegisterAbonentCmd.STATUS_SUBSCRIPTION_LOCKED:
                      sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                      sendMessage(serviceAddress, sourceAddress, msgSubscriptionLocked);
                      break;
                    case SponsoredRegisterAbonentCmd.STATUS_SYSTEM_ERROR:
                      sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
                      break;
                    default:
                      log.error("Unknown result code in subscribe: " + cmd.getStatus());
                  }
                }
              });
              ServiceManager.getInstance().getSponsoredService().execute(cmd);

              return true;

            } catch (NumberFormatException e) {
              log.info("Wrong subscription count");
              sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
              sendMessage(serviceAddress, sourceAddress, msgWrongSubscriptionCount);
            }

          } else if (SPONSORED_UNSUBSCRIBE.matcher(msg).matches()) {
            final SponsoredUnregisterAbonentCmd cmd = new SponsoredUnregisterAbonentCmd();
            cmd.setAbonentAddress(sourceAddress);
            cmd.setSourceId(Command.SOURCE_SMPP);
            cmd.addExecutionObserver(new CommandObserver() {
              public void update(Command command) {
                SponsoredUnregisterAbonentCmd cmd = (SponsoredUnregisterAbonentCmd)command;
                switch (cmd.getStatus()) {
                  case SponsoredUnregisterAbonentCmd.STATUS_SUCCESS:
                    sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                    sendMessage(serviceAddress, sourceAddress, msgUnsubscriptionOk);
                    break;
                  case SponsoredUnregisterAbonentCmd.STATUS_ABONENT_NOT_REGISTERED:
                    sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                    sendMessage(serviceAddress, sourceAddress, msgSourceAbonentNotRegistered);
                    break;
                  case SponsoredUnregisterAbonentCmd.STATUS_SYSTEM_ERROR:
                    sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
                    break;
                  default:
                    log.error("Unknown result code in unsubscribe: " + cmd.getStatus());
                }
              }
            });
            ServiceManager.getInstance().getSponsoredService().execute(cmd);

            return true;

          } else if (msg.matches(SPONSORED + ".*")) {
            log.info("Wrong subscription count");
            sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
            sendMessage(serviceAddress, sourceAddress, msgWrongSubscriptionCount);

            return true;
          }
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

  private void handleDeliveryReport(Message report) {
    final SponsoredRegisterDeliveryCmd cmd = new SponsoredRegisterDeliveryCmd();
    cmd.setDestinationAddress(report.getSourceAddress());
    if (report.getMessageState() == Message.MSG_STATE_DELIVERED)
      ServiceManager.getInstance().getSponsoredService().execute(cmd);
  }
}
