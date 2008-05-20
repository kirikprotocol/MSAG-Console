package ru.sibinco.smsx.engine.service.secret;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.engine.service.secret.datasource.DBSecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;

import java.io.File;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

class SecretServiceImpl implements SecretService {

  private final SecretDataSource dataSource;
  private final SecretProcessor processor;
  private final MessageSender messageSender;

  SecretServiceImpl(XmlConfig config, OutgoingQueue outQueue) {

    try {
      dataSource = new DBSecretDataSource();

      messageSender = new MessageSender(dataSource, outQueue);
      messageSender.setServiceAddress(config.getSection("secret").getString("service.address"));
      messageSender.setMsgDeliveryReport(config.getSection("secret").getString("delivery.report"));
      messageSender.setMsgDestinationAbonentInform(config.getSection("secret").getString("destination.abonent.inform"));
      messageSender.setMsgDestinationAbonentInvitation(config.getSection("secret").getString("destination.abonent.invitation"));

      processor = new SecretProcessor(dataSource, messageSender);

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void execute(SecretChangePasswordCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SecretGetMessagesCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SecretGetMessageStatusCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SecretRegisterAbonentCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SecretSendMessageCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SecretUnregisterAbonentCmd cmd) {
    processor.execute(cmd);
  }

  public boolean execute(SecretHandleReceiptCmd cmd) {
    return processor.execute(cmd);
  }

  public void startService() {
  }

  public void stopService() {
    dataSource.release();
  }
}
