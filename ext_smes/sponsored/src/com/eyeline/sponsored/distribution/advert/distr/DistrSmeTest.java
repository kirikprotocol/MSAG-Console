package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.smppcontainer.SMPPServiceContainer;
import com.eyeline.sme.test.smpp.SimpleResponse;
import com.eyeline.sme.test.smpp.TestMultiplexor;
import com.eyeline.sme.utils.smpp.OutgoingQueue;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.utils.config.xml.XmlConfig;
import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;

/**
 * User: artem
 * Date: 03.02.2008
 */

public class DistrSmeTest extends DistributionSme {

  public DistrSmeTest(XmlConfig config, SmscTimezonesList timezones, OutgoingQueue outQueue) {
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
    private int sn = 0;

    public void sendResponse(PDU pdu) {

    }

    public void sendMessage(Message message) throws SMPPException {
      if (start == -1)
        start = System.currentTimeMillis();

      final SimpleResponse resp = new SimpleResponse(message);
      resp.setSequenceNumber(message.getSequenceNumber());
      resp.setStatus(Data.ESME_ROK);
      resp.setStatusClass(PDU.STATUS_CLASS_NO_ERROR);
      resp.setType(PDU.TYPE_SUBMIT_RESP);
      handleResponse(resp);

      counter++;
      if (counter > 1000) {
        System.out.println("1000");
        counter = 0;
        System.out.println(1000 * 1000 / (System.currentTimeMillis() - start));
        start = -1;
      }

      sn++;

      final Message receipt = new Message();
      receipt.setSourceAddress(message.getDestinationAddress());
      receipt.setDestinationAddress("741");
      receipt.setConnectionName("smsx");
      receipt.setMessageString("");
      receipt.setReceipt(true);
      receipt.setSequenceNumber(sn);
      handleMessage(receipt);
    }

    public void connect() throws SMPPException {

    }

    public void shutdown() {

    }
  }
}
