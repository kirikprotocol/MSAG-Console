package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.secret.commands.*;
import ru.sibinco.smsx.engine.service.secret.datasource.DBSecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;

import java.io.File;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public class SecretServiceImpl implements SecretService {

  private final SecretDataSource dataSource;
  private final SecretProcessor processor;
  private final MessageSender messageSender;
  private final AdvertisingClient advClient;
  private final BatchEngine batchEngine;
  private final DataSourceCleaner dsCleaner;

  public SecretServiceImpl(XmlConfigSection sec, OutgoingQueue outQueue, int serviceId) {

    try {
      dataSource = new DBSecretDataSource();
      advClient = AdvertisingClientFactory.createAdvertisingClient();

      messageSender = new MessageSender(dataSource, outQueue, advClient, serviceId);
      messageSender.setServiceAddress(sec.getString("service.address"));
      messageSender.setMsgDeliveryReport(sec.getString("delivery.report"));
      messageSender.setMsgDestinationAbonentInform(sec.getString("destination.abonent.inform"));
      messageSender.setMsgDestinationAbonentInvitation(sec.getString("destination.abonent.invitation"));
      messageSender.setAdvDelim(sec.getString("advertising.delimiter"));
      messageSender.setAdvSize(sec.getInt("advertising.size"));
      messageSender.setAdvService(sec.getString("advertising.service"));

      File storeDir = new File(sec.getString("store.dir"));
      if (!storeDir.exists() && !storeDir.mkdirs())
        throw new ServiceInitializationException("Can't create store dir: " + storeDir.getAbsolutePath());

      File archivesDir = new File(sec.getString("archives.dir"));
      if (!archivesDir.exists() && !archivesDir.mkdirs())
        throw new ServiceInitializationException("Can't create archives dir: " + archivesDir.getAbsolutePath());

      batchEngine = new BatchEngine(dataSource, storeDir, archivesDir);
      processor = new SecretProcessor(dataSource, messageSender, serviceId);

      dsCleaner = new DataSourceCleaner(dataSource, sec.getInt("ds.clean.interval", 60000), sec.getInt("ds.clean.limit", 100), sec.getInt("ds.clean.maxAliveDays", 30));

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
    dsCleaner.start();
  }

  public void stopService() {
    dsCleaner.stop();
    messageSender.shutdown();
    dataSource.release();
    advClient.close();
    batchEngine.shutdown();
  }

  public Object getMBean(String domain) {
    return new SecretMBean(messageSender);
  }

  public String execute(SecretBatchCmd cmd) throws CommandExecutionException {
    return batchEngine.execute(cmd);
  }

  public int execute(SecretGetBatchStatusCmd cmd) throws CommandExecutionException {
    return batchEngine.execute(cmd);
  }
}
