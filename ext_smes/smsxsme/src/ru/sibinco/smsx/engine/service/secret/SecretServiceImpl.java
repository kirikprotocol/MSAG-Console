package ru.sibinco.smsx.engine.service.secret;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.engine.service.secret.datasource.DBSecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;

import java.io.File;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public class SecretServiceImpl implements SecretService {

  private final SecretDataSource dataSource;
  private final SecretProcessor processor;
  private final MessageSender messageSender;

  public SecretServiceImpl(XmlConfig config, OutgoingQueue outQueue) {

    try {
      dataSource = new DBSecretDataSource();

      XmlConfigSection sec = config.getSection("secret");

      messageSender = new MessageSender(dataSource, outQueue);
      messageSender.setServiceAddress(sec.getString("service.address"));
      messageSender.setMsgDeliveryReport(sec.getString("delivery.report"));
      messageSender.setMsgDestinationAbonentInform(sec.getString("destination.abonent.inform"));
      messageSender.setMsgDestinationAbonentInvitation(sec.getString("destination.abonent.invitation"));

      processor = new SecretProcessor(dataSource, messageSender);

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void execute(SecretChangePasswordCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public void execute(SecretGetMessagesCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public int execute(SecretGetMessageStatusCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void execute(SecretRegisterAbonentCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public long execute(SecretSendMessageCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void execute(SecretUnregisterAbonentCmd cmd) throws CommandExecutionException {
    processor.execute(cmd);
  }

  public boolean execute(SecretHandleReceiptCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void startService() {
  }

  public void stopService() {
    dataSource.release();
  }

  public Object getMBean(String domain) {
    return new SecretMBean(messageSender);
  }
}
