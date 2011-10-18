package ru.novosoft.smsc.admin.perfmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Random;

/**
 * author: Aleksandr Khalitov
 */
public class TestPerfMonitorManager extends PerfMonitorManager{

  public TestPerfMonitorManager(int[] ports) throws AdminException {
    super(new TestPerfMonitorContext(ports));
  }

  @Override
  protected PerfServer createServer(int clientPort, InetAddress smscAddress, final boolean support64Bit) throws IOException {
    return new PerfServer(clientPort, smscAddress, support64Bit){
      @Override
      protected void handleClientSocket(final Socket sock) throws IOException {
        MonitorClientHandler sr = new TestMonitorClientHandler(sock);
        addHandler(sr);
        sr.start();
      }
    };
  }

  private static class TestMonitorClientHandler extends MonitorClientHandler{

    private Socket sock;

    private TestMonitorClientHandler(Socket sock) {
      this.sock = sock;
    }

    @Override
    protected void handle() {
      DataOutputStream os = null;
      try {
        os = new DataOutputStream(sock.getOutputStream());
        snapGenerator(os);
      }catch (Exception e) {
        e.printStackTrace();
      } finally {
        if (os != null)
          try {
            os.close();
          } catch (Exception ignored) {
          }
        if (sock != null)
          try {
            sock.close();
          } catch (Exception ignored) {
          }
      }
    }

    void fillDebugSnap(PerfSnap snap) {
      snap.last[1] = 120;
      snap.last[2] = 5;
      snap.last[3] = 12;
      snap.total[1] = 7825649;
      snap.total[2] = 9202;
      snap.total[3] = 189723;
      snap.uptime = 3600 * 24 * 3 + 3600 * 7 + 60 * 23 + 46;
      snap.sctime = System.currentTimeMillis();
      snap.avg[1] = snap.total[1] / snap.uptime;
      snap.avg[2] = snap.total[2] / snap.uptime;
      snap.avg[3] = snap.total[3] / snap.uptime;
    }

    protected void snapGenerator(DataOutputStream os) throws IOException {
      int scale = 160;
      PerfSnap snap = new PerfSnap(false);
      fillDebugSnap(snap);
      //    int difsuccsess = 50;
      int diferror = 10;
      int difresch = 10;
      Random rand = new Random(System.currentTimeMillis());
      long lastsec = System.currentTimeMillis() / 1000;
      while (!isStopping) {
        long t = System.currentTimeMillis() / 1000;
        if (t <= lastsec) {
          try {
            sleep(50);
          } catch (InterruptedException ignored) {
          }
          continue;
        }
        lastsec = t;
        snap.uptime++;
        snap.sctime++;
        int rnd = rand.nextInt();
        if (rnd < 0) rnd *= -1;
        int ns = (rnd % (scale / 4)) + (scale / 2);
        rnd = rand.nextInt();
        if (rnd < 0) rnd *= -1;
        int ne = rnd % diferror;
        rnd = rand.nextInt();
        if (rnd < 0) rnd *= -1;
        int nr = rnd % difresch;//+10;
        snap.last[1] = ns;
        snap.last[2] = ne;
        snap.last[3] = nr;
        snap.total[1] += ns;
        snap.total[2] += ne;
        snap.total[3] += nr;
        snap.avg[1] = snap.total[1] / snap.uptime;
        snap.avg[2] = snap.total[2] / snap.uptime;
        snap.avg[3] = snap.total[3] / snap.uptime;
        snap.write(os);
        os.flush();
      }
    }
  }
}

