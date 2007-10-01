package ru.sibinco.smsx.tools;

import org.apache.axis.AxisFault;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderResponse;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderStub;

import java.net.MalformedURLException;
import java.net.URL;

/**
 * User: artem
 * Date: 04.07.2007
 */

public class WebSmsTest {
  public static void main(String[] args) {
    for (int i=0; i<1; i++)
      new Worker(i).start();
  }

  private static class Worker extends Thread {
    private final int id;

    private Worker(int id) {
      this.id = id;
    }

    public void run() {
      long start = System.currentTimeMillis();
      int count = 2;

      try {
        final SmsXSenderStub client = new SmsXSenderStub(new URL("http://192.168.1.194:8080/axis/services/SmsXSender"), null);
        client.setMaintainSession(true);

        for (int i=1; i < count; i++) {
          try {
//            SmsXSenderResponse resp = client.sendSms("+79139023974", "sendSmsTest" + id, false, true, false, System.currentTimeMillis() + i * 10000);
            SmsXSenderResponse resp = client.checkStatus("11");
            System.out.println("status = " + resp.getStatus());
            System.out.println("id = " + resp.getId_message());
            System.out.println("smpp = " + resp.getSmpp_code());

//          System.out.println(resp.getId_message() + ": " + resp.getSmpp_code() + ", " + resp.getStatus());
          } catch (AxisFault axisFault) {
            axisFault.printStackTrace();
          } catch (java.rmi.RemoteException e) {
            e.printStackTrace();
          }
        }
      } catch (MalformedURLException e) {
        e.printStackTrace();
      } catch (AxisFault axisFault) {
        axisFault.printStackTrace();
      }
      System.out.println(count * 1000 / (System.currentTimeMillis() - start));
    }
  }
}
