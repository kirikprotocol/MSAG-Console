package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Random;

/**
 * author: Aleksandr Khalitov
 */
public class TestTopMonitorManager extends TopMonitorManager{
  public TestTopMonitorManager(final int[] ports) throws AdminException {
    super(new TestTopMonitorContext(ports));
  }

  @Override
  protected TopServer createServer(int clientPort, InetAddress smscAddress) throws IOException {
    return new TopServer(clientPort, smscAddress){
      @Override
      protected TopServerHandler createClientHandler(Socket sock) throws IOException {
        return new TopServerHandler(sock, this){
          @Override
          protected void initSmscConn() throws IOException {}

          @Override
          protected void _handle(DataOutputStream os) throws IOException{
            Random r = new Random();
            TopSnap topSnap = new TopSnap();
            while(!isStopping) {
              try {
                sleep(1000);
              } catch (InterruptedException ignored) {
              }
              modifySnap(topSnap, r);
              topSnap.write(os);
              os.flush();
            }
          }
        };
      }
    };
  }

  private static void modifySnap(TopSnap snap, Random r) {
    if(snap.errCount == 0) {
      snap.errCount = 2;
      snap.errSnap = new ErrorSnap[2];
      snap.errSnap[0] = new ErrorSnap();
      snap.errSnap[0].avgCount = 0;
      snap.errSnap[1] = new ErrorSnap();
      snap.errSnap[1].code = 1179;
    }
    snap.errSnap[0].count+=r.nextInt(5);
    snap.errSnap[1].count+=r.nextInt(5);
    snap.errSnap[0].avgCount = (short)(r.nextInt(100));
    snap.errSnap[1].avgCount = (short)(r.nextInt(100));
    if(snap.smeCount == 0) {
      snap.smeCount = 0;
      snap.smeSnap = new SmeSnap[2];
      snap.smeSnap[0] = new SmeSnap();
      snap.smeSnap[0].smeId="Sme 1";
      snap.smeSnap[1] = new SmeSnap();
      snap.smeSnap[1].smeId="Sme 2";
    }
    for(int i=0;i<SmeSnap.COUNTERS_NUM;i++) {
      snap.smeSnap[0].speed[i] += r.nextInt(5);
      snap.smeSnap[0].avgSpeed[i] = (short)(r.nextInt(100));
      snap.smeSnap[1].speed[i] += r.nextInt(5);
      snap.smeSnap[1].avgSpeed[i] = (short)(r.nextInt(100));
    }
  }

}
