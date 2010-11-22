package mobi.eyeline.informer.admin.delivery.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * @author Aleksandr Khalitov
 */
public class DcpClient extends SyncProtogenConnection {

  private static final Logger log = Logger.getLogger(DcpClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;


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

//  public boolean isConnected() {
//    return true;
//  }
  
  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    FailResponse fail = new FailResponse();
    try {
      PDU resp;
      resp = request(request, response, fail);
      
      if (resp == fail)
        throw new DeliveryException("interaction_error", fail.getStatus() + "");
      return response;
    } catch (IOException e) {
      throw new DeliveryException("interaction_error", e, e.getMessage());
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

  public void send(UserAuth req) throws AdminException {
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

  private static class ResponseListener {
    private final CountDownLatch respLatch = new CountDownLatch(1);

    private PDU response;
    private final PDU responseEx;

    ResponseListener(PDU responseEx) {
      this.responseEx = responseEx;
    }

    PDU getResponse(int timeout) throws InterruptedException {
      respLatch.await(timeout, TimeUnit.MILLISECONDS);
      return response;
    }

    int getExpectedResponseTag() {
      return responseEx.getTag();
    }

    PDU receive(BufferReader buffer, int tag) throws IOException {
      if (tag == DcpClientTag.FailResponse.getValue()) {
        FailResponse failResponse = new FailResponse();
        failResponse.decode(buffer);
        response = failResponse;
      } else if (getExpectedResponseTag() != tag) {
        log.error("Unexpected tag: " + tag);
        throw new IOException("Unexpected tag: " + tag);
      } else {
        responseEx.decode(buffer);
        response = responseEx;
      }
      respLatch.countDown();
      return response;
    }

  }

}
