package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sme.smpp.test.SimpleResponse;
import com.eyeline.sme.smpp.MessageListener;
import com.eyeline.sme.smpp.MessageSender;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
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
    SMPPTransceiver smppTranceiver = null;
    DistributionSme sme = null;
    MessageHandler handler = null;

    try {
      final XmlConfig config = new XmlConfig(new File("conf/config.xml"));
      config.load();

      Config c= new Config(config);

      final PropertiesConfig smppProps = new PropertiesConfig(c.getSmppConfigFile());

      smppTranceiver = new SMPPTransceiver(new Multiplexor(), smppProps, "");

      handler = new MessageHandler(c.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      final SmscTimezonesList timezones = new SmscTimezonesList(c.getTimezonesFile(), c.getRoutesFile());

      sme = new DistributionSme(config, timezones, smppTranceiver.getOutQueue());

      smppTranceiver.connect();
      handler.start();

    } catch (Exception e) {
      e.printStackTrace();
      sme.stop();
    }
  }

  public static class Multiplexor extends TestMultiplexor {

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
      receipt.setMessageState(Message.MSG_STATE_DELIVERED);
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
