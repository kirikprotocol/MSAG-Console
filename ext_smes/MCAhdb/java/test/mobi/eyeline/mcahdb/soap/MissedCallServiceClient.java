package mobi.eyeline.mcahdb.soap;

import org.apache.axis2.AxisFault;
import org.apache.axis2.engine.AxisConfiguration;
import org.apache.axis2.context.ConfigurationContext;

/**
 * User: artem
 * Date: 05.08.2008
 */

public class MissedCallServiceClient {

  public static void main(String[] args) {
    try {
      final MissedCallServiceStub s = new MissedCallServiceStub("http://localhost:8088/mca/services/MissedCalls");


      long start = System.currentTimeMillis();
      for (int i=1000; i<1000 + 1000; i++) {
        MissedCallServiceStub.GetMissedCalls req = new MissedCallServiceStub.GetMissedCalls();
        req.setPhoneNumber("+7913903" + i);

//        System.out.println("send");
        MissedCallServiceStub.GetMissedCallsResponse resp = s.GetMissedCalls(req);

        MissedCallServiceStub.ArrayOfMissedCall result = resp.getGetMissedCallsResult();
//        if (result.getMissedCall() != null)
//          for (MissedCallServiceStub.MissedCall mc : result.getMissedCall())
//            System.out.println(mc.getPhoneNumber() + " " + mc.getMissedCalls() + " " + mc.getMessageStatus());
//        System.out.println("");
      }

      System.out.println(1000 * 1000 / (System.currentTimeMillis() - start));
      s.cleanup();
    } catch (AxisFault axisFault) {
      axisFault.printStackTrace();
    } catch (java.rmi.RemoteException e) {
      e.printStackTrace();
    }
  }
}
