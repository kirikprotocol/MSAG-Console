package ru.sibinco.smsx.engine.service.sender;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfigSection;

/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderServiceImpl implements SenderService{

  private final SenderProcessor processor;

  public SenderServiceImpl(XmlConfigSection config, OutgoingQueue outQueue, int umrSyffix) {
    try {
      processor = new SenderProcessor(outQueue, config.getInt("cache.size", 10000), umrSyffix);
    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public int execute(SenderGetMessageStatusCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void execute(SenderSendMessageCmd cmd) {
    processor.execute(cmd);
  }

  public boolean execute(SenderHandleReceiptCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void startService() {
  }

  public void stopService() {
  }

  public Object getMBean(String domain) {
    return null;
  }
}
