package mobi.eyeline.informer.admin.delivery.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.protogen.ServerOfflineException;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class DcpClient extends SyncProtogenConnection {

  private static final Logger log = Logger.getLogger(DcpClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;

  private final Lock lock = new ReentrantLock();
  private final String login;
  private final String password;

  public DcpClient(String host, int port, String login, String password) {
    super(host, port, RESPONSE_TIMEOUT);
    this.login = login;
    this.password = password;
  }

  @Override
  protected void onConnect() throws IOException {
    UserAuth auth = new UserAuth();
    auth.setUserId(login);
    auth.setPassword(password);
    try {
      send(auth);
    } catch (AdminException e) {
      log.error(e, e);
      throw new IOException(e.getMessage());
    }
  }

  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    FailResponse fail = new FailResponse();
    try {
      lock.lock();
      if (log.isDebugEnabled())
        log.debug("Dcp Connection locked: login=" + login);

      PDU resp;
      resp = request(request, response, fail);
      
      if (resp == fail)
        throw new DeliveryException("interaction_error", "Status " + fail.getStatus() + ": " + fail.getStatusMessage());
      return response;
    } catch (ServerOfflineException e) {
      throw new DeliveryException("dcp_server_offline");
    } catch (IOException e) {
      throw new DeliveryException("interaction_error", e, e.getMessage());
    } finally {
      lock.unlock();
      if (log.isDebugEnabled())
        log.debug("Dcp Connection unlocked: login=" + login);
    }
  }

  public AddDeliveryMessagesResp send(AddDeliveryMessages req) throws AdminException {
    return sendPdu(req, new AddDeliveryMessagesResp());
  }

  public CreateDeliveryResp send(CreateDelivery req) throws AdminException {
    return sendPdu(req, new CreateDeliveryResp());
  }

  public CountDeliveriesResp send(CountDeliveries req) throws AdminException {
    return sendPdu(req, new CountDeliveriesResp());
  }

  public CountMessagesResp send(CountMessages req) throws AdminException {
    return sendPdu(req, new CountMessagesResp());
  }

  void send(UserAuth req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  public GetUserStatsResp send(GetUserStats req) throws AdminException {
    return sendPdu(req, new GetUserStatsResp());
  }

  public void send(ModifyDelivery req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  public void send(DropDelivery req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  public void send(ChangeDeliveryState req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  public void send(DropDeliveryMessages req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  public GetDeliveryGlossaryResp send(GetDeliveryGlossary req) throws AdminException {
    return sendPdu(req, new GetDeliveryGlossaryResp());
  }

  public void send(ModifyDeliveryGlossary req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

  public GetDeliveryStateResp send(GetDeliveryState req) throws AdminException {
    return sendPdu(req, new GetDeliveryStateResp());
  }

  public GetDeliveryInfoResp send(GetDeliveryInfo req) throws AdminException {
    return sendPdu(req, new GetDeliveryInfoResp());
  }

  public GetDeliveriesListResp send(GetDeliveriesList req) throws AdminException {
    return sendPdu(req, new GetDeliveriesListResp());
  }

  public GetDeliveriesListNextResp send(GetDeliveriesListNext req) throws AdminException {
    return sendPdu(req, new GetDeliveriesListNextResp());
  }

  public RequestMessagesStateResp send(RequestMessagesState req) throws AdminException {
    return sendPdu(req, new RequestMessagesStateResp());
  }

  public GetNextMessagesPackResp send(GetNextMessagesPack req) throws AdminException {
    return sendPdu(req, new GetNextMessagesPackResp());
  }

  public GetDeliveryHistoryResp send(GetDeliveryHistory req) throws AdminException {
    return sendPdu(req, new GetDeliveryHistoryResp());
  }

}
