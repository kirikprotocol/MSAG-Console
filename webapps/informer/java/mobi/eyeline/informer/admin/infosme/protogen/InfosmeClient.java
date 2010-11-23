package mobi.eyeline.informer.admin.infosme.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.InfosmeException;
import mobi.eyeline.informer.admin.infosme.OfflineException;
import mobi.eyeline.informer.admin.infosme.protogen.protocol.*;
import mobi.eyeline.informer.admin.protogen.ServerOfflineException;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.protogen.framework.PDU;

import java.io.IOException;

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

  ConfigOpResult send(ConfigOp req) throws AdminException{
    return sendPdu(req, new ConfigOpResult());
  }

  protected LoggerGetCategoriesResp send(LoggerGetCategories req) throws AdminException{
    return sendPdu(req, new LoggerGetCategoriesResp());
  }

  protected LoggerSetCategoriesResp send(LoggerSetCategories req) throws AdminException {
    return sendPdu(req, new LoggerSetCategoriesResp());
  }

  SetDefaultSmscResp send(SetDefaultSmsc req) throws AdminException {
    return sendPdu(req, new SetDefaultSmscResp());
  }

  SendTestSmsResp send(SendTestSms req) throws AdminException {
    return sendPdu(req, new SendTestSmsResp());
  }

  public void shutdown() {
    close();
  }
}
