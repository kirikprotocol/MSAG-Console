package com.eyeline.sponsored;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.SimpleResponse;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sponsored.config.Config;
import com.eyeline.sponsored.distribution.advert.distr.DistributionSme;
import com.eyeline.sponsored.subscription.service.SubscriptionSme;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;

import java.io.File;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class Sme {

  public static void main(String[] args) {

    MessageHandler handler = null;
    SMPPTransceiver smppTranceiver = null;
    SubscriptionSme subscriptionSme = null;
    DistributionSme distributionSme = null;

    try {
      final XmlConfig xmlConfig = new XmlConfig(new File("conf/config.xml"));
      xmlConfig.load();

      final Config conf = new Config(xmlConfig);

      final SmscTimezonesList timezones = new SmscTimezonesList(conf.getTimezonesFile(), conf.getRoutesFile());

      final PropertiesConfig smppProps = new PropertiesConfig(conf.getSmppConfigFile());

      if (args.length > 0 && args[0].equals("-t")) {
        System.out.println("Sponsored started in test mode");
        smppTranceiver = new SMPPTransceiver(new Multiplexor(), smppProps, "");
      } else
        smppTranceiver = new SMPPTransceiver(smppProps, "");

      subscriptionSme = new SubscriptionSme(xmlConfig, timezones);
      distributionSme = new DistributionSme(xmlConfig, timezones, smppTranceiver.getOutQueue());

      handler = new MessageHandler(conf.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      smppTranceiver.connect();
      handler.start();

      new ConfigUpdater(600000, timezones).start();

    } catch (Throwable e) {
      e.printStackTrace();
      subscriptionSme.stop();
      distributionSme.stop();
    }
  }

  protected static class ConfigUpdater extends Thread {
    private static final Category log = Category.getInstance("DISTRIBUTION");

    private final long delay;
    private final SmscTimezonesList timezones;

    public ConfigUpdater(long delay, SmscTimezonesList timezones) {
      super("ConfReloadThread");
      this.delay = delay;
      this.timezones = timezones;
    }

    public synchronized void run() {
      try {
        wait(delay);
      } catch (InterruptedException e) {
      }

      try {
        timezones.reload();
      } catch (SmscTimezonesListException e) {
        log.error("Timezones reload failed", e);
      }

      if (log.isDebugEnabled())
        log.debug("Timezones have been reloaded.");
    }
  }

  protected static class InitException extends RuntimeException {

    public InitException(String message) {
      super(message);
    }

    public InitException(Throwable cause) {
      super(cause);
    }
  }

  private static class Multiplexor extends TestMultiplexor {

    private int count;

    public void sendResponse(PDU pdu) {

    }

    public void sendMessage(Message message) throws SMPPException {
      SimpleResponse simpleResponse = new SimpleResponse(message);
      simpleResponse.setStatus(Data.ESME_ROK);
      simpleResponse.setStatusClass(PDU.STATUS_CLASS_NO_ERROR);
      handleResponse(simpleResponse);

      final Message receipt = new Message();
      receipt.setSourceAddress(message.getDestinationAddress());
      receipt.setDestinationAddress("741");
      receipt.setConnectionName("smsx");
      receipt.setMessageState(Message.MSG_STATE_DELIVERED);
      receipt.setMessageString("");
      receipt.setReceipt(true);
      receipt.setSequenceNumber(count++);
      handleMessage(receipt);
    }

    public void connect() throws SMPPException {

    }

    public void shutdown() {

    }
  }
}
