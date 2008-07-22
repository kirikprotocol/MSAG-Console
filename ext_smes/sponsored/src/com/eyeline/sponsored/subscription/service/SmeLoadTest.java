package com.eyeline.sponsored.subscription.service;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.SimpleResponse;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sponsored.subscription.config.Config;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;

/**
 *
 * @author artem
 */
public class SmeLoadTest extends SubscriptionSme {

  public SmeLoadTest(XmlConfig config, SmscTimezonesList timezones) {
    super(config, timezones);
  }

  public static void main(String[] args) {
    SMPPTransceiver smppTranceiver = null;
    SubscriptionSme sme = null;
    MessageHandler handler = null;

    try {
      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

      Config conf = new Config(xmlConfig);

      final PropertiesConfig smppProps = new PropertiesConfig();
      smppProps.load(new File(conf.getSmppConfigFile()));

      smppTranceiver = new SMPPTransceiver(new Multiplexor(), smppProps, "");

      handler = new MessageHandler(conf.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      final SmscTimezonesList timezones = new SmscTimezonesList();
      timezones.load(conf.getTimezonesFile(), conf.getRoutesFile());

      sme = new SubscriptionSme(xmlConfig, timezones);

      smppTranceiver.connect();
      handler.start();

    } catch (Exception e) {
      e.printStackTrace();
      sme.stop();
    }
  }

  private static class Multiplexor extends TestMultiplexor {

    private final SyncWorker worker = new SyncWorker();
    private final AsyncWorker worker1 = new AsyncWorker();

    public void sendResponse(PDU response) {
      if (response.getStatus() == Data.ESME_RTHROTTLED) {
        System.out.println("Throlled!!!");
      }
    }

    public void _sendMessage(Message message) throws SMPPException {

      worker.wait = false;
      final SimpleResponse resp = new SimpleResponse(message);
      resp.setSequenceNumber(message.getSequenceNumber());
      resp.setStatus(Data.ESME_ROK);
      resp.setStatusClass(PDU.STATUS_CLASS_NO_ERROR);
      resp.setType(PDU.TYPE_SUBMIT_RESP);
      handleResponse(resp);
    }

    public void connect() throws SMPPException {
      super.connect();
      //worker.start();
      worker1.start();
    }

    public void shutdown() {
      super.shutdown();
    }

    private class SyncWorker extends Thread {

      private boolean wait = false;
      private final Object lock = new Object();

      public void run() {
        long start = System.currentTimeMillis();
        int s = 100000, e = 1000000;
        int n = 0;
        int seqNum = 0;
        for (int i = s; i < e; i++, seqNum++) {
          final Message m = new Message();
          m.setSourceAddress("+79169" + i);
          m.setDestinationAddress("741");
          m.setMessageString("10");
          m.setConnectionName("smsx");
          m.setSequenceNumber(seqNum);

          wait = true;
          handleMessage(m);
          while (wait) {
            ;
          }

//          synchronized (lock) {
//            try {
//              lock.wait(1);
//            } catch (Throwable ex) {
//
//            }
//          }

          n++;
          if (n == 10000) {
            n = 0;
            System.out.println(10000 * 1000 / (System.currentTimeMillis() - start));
            start = System.currentTimeMillis();
          }
        }
        System.out.println((e - s) * 1000 / (System.currentTimeMillis() - start));
      }
    }

    private class AsyncWorker extends Thread {

      private final Object lock = new Object();

      public void run() {
        long start = System.currentTimeMillis();
        int s = 100000, e = 1000000;
        int n = 0;
        int seqNum = 0;
        for (int i = s; i < e; i++, seqNum++) {
          final Message m = new Message();
          m.setSourceAddress("+79121" + i);
          m.setDestinationAddress("741");
          m.setMessageString("20");
          m.setConnectionName("smsx");
          m.setSequenceNumber(seqNum);

          handleMessage(m);

          synchronized (lock) {
            try {
              lock.wait(3);
            } catch (Throwable ex) {

            }
          }

          n++;
          if (n == 10000) {
            n = 0;
            System.out.println("Async = " + 10000 * 1000 / (System.currentTimeMillis() - start));
            start = System.currentTimeMillis();
          }
        }
        System.out.println((e - s) * 1000 / (System.currentTimeMillis() - start));
      }
    }
  }
  }
  
