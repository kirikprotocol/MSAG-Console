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

public class CheckStatus {
  public static void main(String[] args) {
    if (args.length < 2) {
      System.out.println("Usage:");
      System.out.println("  Arg[0] = URL");
      System.out.println("  Arg[1] = message id");
      return;
    }

    try {
      final SmsXSenderStub client = new SmsXSenderStub(new URL(args[0]), null);
      client.setMaintainSession(true);

      SmsXSenderResponse resp = client.checkStatus(args[1]);
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
