import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.protogen.framework.*;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class UpdateConfigClient extends SyncProtogenConnection {

  private static final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

  public static final int RESPONSE_TIMEOUT = 30000;

  public UpdateConfigClient(String host, int port){
      super(host, port, RESPONSE_TIMEOUT);
  }

  private static final Logger log = Logger.getLogger(UpdateConfigClient.class);

  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    try {
        request(request, response);
    } catch (IOException e) {
        log.error(e);
    }
    return response;
  }

  static public void main(String[] a) throws AdminException {
      Properties config = new Properties();
        try {

          String userDir = System.getProperty("user.dir");
          String filename = userDir+ File.separator+"conf"+File.separator+"config.properties";

          config.load(new FileReader(filename));
          log.debug("Successfully load config properties.");

          String s = config.getProperty("host");
          String host = null;
          if (s != null && !s.isEmpty()){
              host = config.getProperty("host");
              log.debug("host="+host);
          } else {
              log.error("Check 'host' property.");
              System.exit(1);
          }

          int port = 0;
          s = config.getProperty("port");
          if (s != null && !s.isEmpty()){
              port = Integer.parseInt("port");
              log.debug("port="+port);
          } else {
              log.error("Check 'port' property.");
              System.exit(2);
          }

          final UpdateConfigClient updateConfigClient = new UpdateConfigClient(host, port);

          scheduler.scheduleAtFixedRate(new Runnable() {

              @Override
              public void run() {
                  try {
                      updateConfigClient.send(new UpdateConfig());
                  } catch (AdminException e) {
                      log.error(e);
                      System.exit(4);
                  }
              }

          }, 5, 5, TimeUnit.SECONDS);

        }catch (IOException e) {
          log.error(e);
          System.exit(3);
      }


  }

  public UpdateConfigResp send(UpdateConfig req) throws AdminException {
      return sendPdu(req, new UpdateConfigResp());
  }


  @Override
  protected void onConnect() throws IOException, AdminException {
        //To change body of implemented methods use File | Settings | File Templates.
  }


}
