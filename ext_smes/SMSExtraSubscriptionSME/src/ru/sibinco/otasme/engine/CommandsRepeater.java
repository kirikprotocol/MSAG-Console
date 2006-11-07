package ru.sibinco.otasme.engine;

import ru.sibinco.otasme.utils.Service;
import ru.sibinco.otasme.utils.Utils;
import ru.sibinco.otasme.network.OutgoingObject;
import ru.sibinco.otasme.Sme;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.util.Vector;
import java.util.Properties;

/**
 * User: artem
 * Date: 02.11.2006
 */

public final class CommandsRepeater extends Service {
  private final static Category log = Category.getInstance(CommandsRepeater.class);
  private final static String OTA_NUMBER;
  private final static long RETRY_PERIOD;

  private static final Commands2Send commands2send;

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    OTA_NUMBER = Utils.loadString(config, "sme.engine.ota.number");
    RETRY_PERIOD = Utils.loadLong(config, "commands.repeater.retry.period");

    commands2send = new Commands2Send(Utils.loadInt(config, "commands.repeater.size"));
    new CommandsRepeater().startService();
  }

  public static void addCommand(Command2Send command) throws QueueOverflowException {
    commands2send.put(command);
  }



  // IMPLEMENTATION

  private CommandsRepeater() {
    super(log);
  }

  public synchronized void iterativeWork() {
    final Vector commands = commands2send.getCommands();
    for (int i=0; i<commands.size(); i++)
      sendCommand((Command2Send)commands.get(i));

    try {
      wait(RETRY_PERIOD);
    } catch (InterruptedException e) {
      log.error("Interrupted", e);
    }
  }

  private static void sendCommand(Command2Send command2send) {
    final Message otaRequest = new Message();
    otaRequest.setType(Message.TYPE_WTS_REQUEST);
    otaRequest.setWtsOperationCode(Message.WTS_OPERATION_CODE_COMMAND);
    otaRequest.setWTSUserId(command2send.getWtsUserId());
    otaRequest.setWTSServiceName(command2send.getWtsServiceName());
    otaRequest.setSourceAddress(OTA_NUMBER);
    otaRequest.setDestinationAddress(OTA_NUMBER);
    otaRequest.setWtsRequestReference(command2send.getWtsRequestReference());
    Sme.outQueue.addOutgoingObject(new OutgoingObject(otaRequest));
    log.info("COMMANDS REPEATER: Send SR_COMMAND from abonent = " + command2send.getWtsUserId() + " with service name = " + command2send.getWtsServiceName());
  }
}
