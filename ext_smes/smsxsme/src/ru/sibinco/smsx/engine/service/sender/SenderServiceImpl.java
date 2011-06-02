package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.service.sender.datasource.DBSenderDataSource;
import ru.sibinco.smsx.engine.service.sender.datasource.MultiTableSenderDS;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderDataSource;

/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderServiceImpl implements SenderService{

  private final SenderDataSource dataSource;
  private final MessageSender senderMessage;
  private final SenderProcessor processor;

  public SenderServiceImpl(XmlConfigSection config, OutgoingQueue outQueue, int serviceId) {
    try {
      String type = config.getString("dataSourceType", "singleTable");
      dataSource = type.equals("multiTable") ? new MultiTableSenderDS() : new DBSenderDataSource();

      senderMessage = new MessageSender(outQueue, dataSource, serviceId);

      processor = new SenderProcessor(dataSource, senderMessage, serviceId);

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
