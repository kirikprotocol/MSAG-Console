package mobi.eyeline.informer.admin.infosme.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.InfosmeException;
import mobi.eyeline.informer.admin.infosme.OfflineException;
import mobi.eyeline.informer.admin.infosme.protogen.protocol.*;
import mobi.eyeline.informer.admin.protogen.ServerOfflineException;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.ClientConnection;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * @author Aleksandr Khalitov
 */
class InfosmeClient extends SyncProtogenConnection {

  private static final int RESPONSE_TIMEOUT = 5000;

  InfosmeClient(String host, int port) {
    super(host, port, RESPONSE_TIMEOUT);
  }

  protected void onConnect() throws IOException {
  }


  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    try {
      return (T)request(request, response);
    } catch (ServerOfflineException e) {
      throw new OfflineException("server_offline");
    } catch (IOException e) {
      throw new InfosmeException("interaction_error", e, e.getMessage());
    }
  }

  protected ConfigOpResult send(ConfigOp req) throws AdminException{
    return sendPdu(req, new ConfigOpResult());
  }

  protected LoggerGetCategoriesResp send(LoggerGetCategories req) throws AdminException{
    return sendPdu(req, new LoggerGetCategoriesResp());
  }

  protected LoggerSetCategoriesResp send(LoggerSetCategories req) throws AdminException {
    return sendPdu(req, new LoggerSetCategoriesResp());
  }

  protected SetDefaultSmscResp send(SetDefaultSmsc req) throws AdminException {
    return sendPdu(req, new SetDefaultSmscResp());
  }

  protected SendTestSmsResp send(SendTestSms req) throws AdminException {
    return sendPdu(req, new SendTestSmsResp());
  }

  public void shutdown() {
    close();
  }
}
