package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.protogen.ResponseWaitTimeoutException;
import mobi.eyeline.informer.admin.protogen.ServerOfflineException;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.informer.admin.smppgw.protogen.protocol.UpdateConfig;
import mobi.eyeline.informer.admin.smppgw.protogen.protocol.UpdateConfigResp;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;

/**
 * author: Aleksandr Khalitov
 */
class SmppGWClient extends SyncProtogenConnection {

  private static final int RESPONSE_TIMEOUT = 30000;

  SmppGWClient(String host, int port, Logger logger) {
    super(host, port, RESPONSE_TIMEOUT, logger);
    if(logger.isDebugEnabled()) {
      logger.debug("SmppGW client started: host="+host+" port="+port);
    }
  }

  protected void onConnect() throws IOException {
  }


  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    try {
      return request(request, response);
    } catch (ServerOfflineException e) {
      throw new OfflineException("server_offline");
    } catch (ResponseWaitTimeoutException e) {
      throw new SmppGWException("response_timeout");
    } catch (IOException e) {
      throw new SmppGWException("io_error", e, e.getMessage());
    }
  }

  UpdateConfigResp send(UpdateConfig req) throws AdminException{
    return sendPdu(req, new UpdateConfigResp());
  }

  public void shutdown() {
    close();
  }
}
