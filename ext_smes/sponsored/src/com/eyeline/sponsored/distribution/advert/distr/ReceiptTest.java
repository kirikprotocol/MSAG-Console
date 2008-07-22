package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;

/**
 * User: artem
 * Date: 03.02.2008
 */

public class ReceiptTest extends DistributionSme {

  public ReceiptTest(XmlConfig config, SmscTimezonesList timezones, OutgoingQueue outQueue) {
    super(config, timezones, outQueue);
  }

  public static void main(String[] args) {
    SMPPTransceiver smppTranceiver = null;
    DistributionSme sme = null;
    MessageHandler handler = null;

    try {
      final XmlConfig config = new XmlConfig();
      config.load(new File("conf/config.xml"));

      Config c= new Config(config);

      final PropertiesConfig smppProps = new PropertiesConfig();
      smppProps.load(new File(c.getSmppConfigFile()));

      smppTranceiver = new SMPPTransceiver(new Multiplexor(), smppProps, "");

      handler = new MessageHandler(c.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      final SmscTimezonesList timezones = new SmscTimezonesList();
      timezones.load(c.getTimezonesFile(), c.getRoutesFile());

      sme = new DistributionSme(config, timezones, smppTranceiver.getOutQueue());

      smppTranceiver.connect();
      handler.start();

    } catch (Exception e) {
      e.printStackTrace();
      sme.stop();
    }
  }

  private static class Multiplexor extends TestMultiplexor {

    private long start = -1;
    private int counter = 0;

    public void sendResponse(PDU pdu) {

    }

    public void _sendMessage(Message message) throws SMPPException {
    }

    public void connect() throws SMPPException {
      int count = 0;
      for (int j=0; j<10; j++) {
        for (int i=1100; i< 1200; i++) {
          final Message receipt = new Message();
          receipt.setSourceAddress("+7913902" + i);
          receipt.setDestinationAddress("741");
          receipt.setConnectionName("smsx");
          receipt.setMessageState(Message.MSG_STATE_DELIVERED);
          receipt.setMessageString("");
          receipt.setReceipt(true);
          receipt.setSequenceNumber(count);
          handleMessage(receipt);
          count++;
        }
      }
      System.out.println("count = " + count);
    }

    public void shutdown() {

    }
  }
}
