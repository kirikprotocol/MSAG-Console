package ru.sibinco.smsx;

import org.apache.axis.AxisFault;
import ru.sibinco.smsx.engine.soaphandler.blacklist.BlacklistSoapStub;

import java.net.MalformedURLException;
import java.net.URL;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class AddToBlackList {
  public static void main(String[] args) {
    if (args.length < 2) {
      System.out.println("Usage:");
      System.out.println("  Arg[0] = URL");
      System.out.println("  Arg[1] = msisdn");
      return;
    }

    try {
      final BlacklistSoapStub client = new BlacklistSoapStub(new URL(args[0]), null);
      client.setMaintainSession(true);

      boolean resp = client.add(args[1]);
      System.out.println("result=" + resp);

    } catch (MalformedURLException e) {
      e.printStackTrace();
    } catch (AxisFault axisFault) {
      axisFault.printStackTrace();
    } catch (java.rmi.RemoteException e) {
      e.printStackTrace();
    }
  }
}
