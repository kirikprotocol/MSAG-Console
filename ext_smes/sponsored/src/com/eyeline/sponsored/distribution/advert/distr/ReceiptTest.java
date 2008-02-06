package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.smppcontainer.SMPPServiceContainer;
import com.eyeline.sme.test.smpp.TestMultiplexor;
import com.eyeline.sme.utils.smpp.OutgoingQueue;
import com.eyeline.sponsored.distribution.advert.config.Config;
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
    SMPPServiceContainer container = null;
    DistributionSme sme = null;
    try {

      final XmlConfig config = new XmlConfig(new File("conf/config.xml"));
      config.load();

      Config c= new Config(config);

      final SmscTimezonesList timezones = new SmscTimezonesList(c.getTimezonesFile(), c.getRoutesFile());

      container = new SMPPServiceContainer(new Multiplexor());
      container.init(c.getContainerConfigFile(), c.getSmppConfigFile());

      sme = new DistributionSme(config, timezones, container.getOutgoingQueue());

      container.start();

    } catch (Exception e) {
      e.printStackTrace();
      container.stop();
      sme.stop();
    }
  }

  private static class Multiplexor extends TestMultiplexor {

    private long start = -1;
    private int counter = 0;

    public void sendResponse(PDU pdu) {

    }

    public void sendMessage(Message message) throws SMPPException {
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
