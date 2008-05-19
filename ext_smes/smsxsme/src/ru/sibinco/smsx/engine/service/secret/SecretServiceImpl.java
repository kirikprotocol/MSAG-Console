package ru.sibinco.smsx.engine.service.secret;

import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.engine.service.secret.datasource.DBSecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;

import java.io.File;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.properties.PropertiesConfig;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

class SecretServiceImpl implements SecretService {

  private final SecretDataSource dataSource;
  private final SecretProcessor processor;
  private final MessageSender messageSender;

  SecretServiceImpl(String configDir, OutgoingQueue outQueue) {

    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "services/secret/service.properties"));

      dataSource = new DBSecretDataSource(new File(configDir, "services/secret/secret.sql").getAbsolutePath(), "");

      messageSender = new MessageSender(dataSource, outQueue);
      messageSender.setServiceAddress(config.getString("service.address"));
      messageSender.setMsgDeliveryReport(config.getString("delivery.report"));
      messageSender.setMsgDestinationAbonentInform(config.getString("destination.abonent.inform"));
      messageSender.setMsgDestinationAbonentInvitation(config.getString("destination.abonent.invitation"));

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
