package ru.sibinco.smsx;

import com.eyeline.utils.config.Arguments;
import com.eyeline.utils.config.ConfigException;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderStub;

import java.net.URL;
import java.net.MalformedURLException;

/**
 * User: artem
 * Date: 06.06.2008
 */

public class SmsXSenderLoadTest {

  public static void main(String[] arguments) {
    final Arguments args = new Arguments(arguments);

    try {
      int sendSpeed = args.getIntAttr("-speed", 10);
      final boolean calendarTest = args.containsAttr("-calendar");
      int threadCount = args.getIntAttr("-threads", 10);

      final int delay = 1000 / sendSpeed;

      final String url = args.getStringParam(0);

      for (int i=0; i < threadCount; i++) {
        new Thread(new Runnable() {
          public void run() {
            try {
              final SmsXSenderStub client = new SmsXSenderStub(new URL(url), null);              
              client.setMaintainSession(true);

              final Object o = new Object();

              while(true) {
                long start = System.currentTimeMillis();
                for (int i=100000; i<1000000; i++) {
                  String msisdn = "+79139" + i;

                  client.sendSms(msisdn, "test", false, false, calendarTest, System.currentTimeMillis() + 10000, false);

                  synchronized(o) {
                    o.wait(delay);
                  }

                  if (i%100 == 0) {
                    System.out.println("Actual speed = " + 100 * 1000 / (System.currentTimeMillis() - start));
                    start = System.currentTimeMillis();
                  }
                }
              }
            } catch (Throwable e) {
              e.printStackTrace();
            }
          }
        }, "Sender-" + i).start();
      }
    } catch (Throwable e) {
      e.printStackTrace();
    }
  }
}
