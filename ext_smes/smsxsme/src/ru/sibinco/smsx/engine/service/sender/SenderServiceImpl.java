package ru.sibinco.smsx.engine.service.sender;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.sender.datasource.DBSenderDataSource;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderDataSource;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;

import java.io.File;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderServiceImpl implements SenderService{

  private final SenderDataSource dataSource;
  private final MessageSender senderMessage;
  private final SenderProcessor processor;

  public SenderServiceImpl(XmlConfig config, OutgoingQueue outQueue) {
    try {
      dataSource = new DBSenderDataSource();

      senderMessage = new MessageSender(outQueue, dataSource);

      processor = new SenderProcessor(dataSource, senderMessage);

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
    dataSource.release();
  }

  public Object getMBean(String domain) {
    return new SenderMBean(senderMessage);
  }
}
