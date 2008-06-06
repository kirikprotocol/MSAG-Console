package ru.sibinco.smsx;

import org.apache.axis.AxisFault;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderResponse;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderStub;

import java.net.MalformedURLException;
import java.net.URL;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class SendSMSMsg {

  public static void main(String[] args) {
    if (args.length < 3) {
      System.out.println("Usage:");
      System.out.println("  Arg[0] = URL");
      System.out.println("  Arg[1] = destination address");
      System.out.println("  Arg[2] = message");
      System.out.println("  Arg[3] = express flug (1)");
      System.out.println("  Arg[4] = append advertising flug (1)");
      return;
    }

    try {
      final SmsXSenderStub client = new SmsXSenderStub(new URL(args[0]), null);
      client.setMaintainSession(true);

      SmsXSenderResponse resp = client.sendSms(args[1], args[2], args.length >=4 && args[3].equals("1"), false, false, 0, args.length >=5 && args[4].equals("1"));
      System.out.println("status = " + Statuses.getStatus(resp.getStatus()));
      System.out.println("id = " + resp.getId_message());
      System.out.println("smpp = " + resp.getSmpp_code());

    } catch (MalformedURLException e) {
      e.printStackTrace();
    } catch (AxisFault axisFault) {
      axisFault.printStackTrace();
    } catch (java.rmi.RemoteException e) {
      e.printStackTrace();
    }
  }
}
