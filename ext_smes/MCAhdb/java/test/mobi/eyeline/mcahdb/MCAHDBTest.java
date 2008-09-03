package mobi.eyeline.mcahdb;

import com.eyeline.utils.config.ConfigException;
import mobi.eyeline.mcahdb.engine.TestUtils;
import mobi.eyeline.mcahdb.soap.MissedCallServiceStub;
import org.apache.axis2.AxisFault;

import java.io.IOException;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * User: artem
 * Date: 02.09.2008
 */

public class MCAHDBTest {

  public static void main(String args[]) {
    ScheduledExecutorService s = Executors.newScheduledThreadPool(2);
    s.scheduleAtFixedRate(new JournalsGenerator(), 0, 60, TimeUnit.SECONDS);
    s.scheduleWithFixedDelay(new GetMissedCalls(args[0]), 60, 10, TimeUnit.SECONDS);
  }

  private static class GetMissedCalls implements Runnable {

    private final String url;

    private GetMissedCalls(String url) {
      this.url = url;
    }

    public void run() {
      MissedCallServiceStub s = null;
      try {
        s = new MissedCallServiceStub(url);

        for (int i=1000; i<1000 + 1000; i++) {
          MissedCallServiceStub.GetMissedCalls req = new MissedCallServiceStub.GetMissedCalls();
          req.setPhoneNumber("+7913903" + i);

          System.out.println("Get missed calls for " + req.getPhoneNumber());
          s.GetMissedCalls(req);

          Thread.sleep(500);
        }
      } catch (AxisFault axisFault) {
        axisFault.printStackTrace();
      } catch (java.rmi.RemoteException e) {
        e.printStackTrace();
      } catch (InterruptedException e) {
        e.printStackTrace();
      } finally {
        if (s != null)
          try {
            s.cleanup();
          } catch (AxisFault axisFault) {
          }
      }
    }
  }

  private static class JournalsGenerator implements Runnable {

    public void run() {
      try {
        GlobalConfig config = new GlobalConfig();
        System.out.println("Generate journal.");
        TestUtils.generateJournal(config.getJournalsStoreDir(), 10000, 10);
      } catch (IOException e) {
        e.printStackTrace();
      } catch (ConfigException e) {
        e.printStackTrace();
      }
    }
  }

}
