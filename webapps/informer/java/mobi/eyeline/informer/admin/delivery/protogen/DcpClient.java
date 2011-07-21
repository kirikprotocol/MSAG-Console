package mobi.eyeline.informer.admin.delivery.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.monitoring.MBean;
import mobi.eyeline.informer.admin.protogen.ResponseWaitTimeoutException;
import mobi.eyeline.informer.admin.protogen.ServerOfflineException;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Синхронный клиент для Dcp
 *
 * @author Aleksandr Khalitov
 */
public class DcpClient extends SyncProtogenConnection {

  public static final int RESPONSE_TIMEOUT = 30000;

  private final Lock lock = new ReentrantLock();
  private final String login;
  private final String password;

  public DcpClient(String host, int port, String login, String password, Logger log) {
    super(host, port, RESPONSE_TIMEOUT, log);
    this.login = login;
    this.password = password;
  }

  @Override
  protected void onConnect() throws AdminException{
    UserAuth auth = new UserAuth();
    auth.setUserId(login);
    auth.setPassword(password);
    send(auth);
  }

  private static MBean getMBean() {
    return MBean.getInstance(MBean.Source.DCP);
  }

  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    FailResponse fail = new FailResponse();
    try {
      lock.lock();
      if (logger.isDebugEnabled())
        logger.debug("Dcp Connection locked: login=" + login);

      PDU resp;
      resp = request(request, response, fail);
      
      if (resp == fail) {
        if(fail.hasStatus()) {
          String message = fail.hasStatusMessage() ?  fail.getStatusMessage() : "";
          DeliveryException.ErrorStatus e = DeliveryException.ErrorStatus.valueOf(fail.getStatus());
          if(e == null) {
            logger.warn("Unknown response status="+fail.getStatus());
          }else {
            if(e == DeliveryException.ErrorStatus.CommandHandling && message.equals("Not ready yet")) {
              throw new DeliveryException("not_ready_yet");
            }
          }
          throw new DeliveryException(e == null ? DeliveryException.ErrorStatus.Unknown : e, message);
        }
        throw new DeliveryException("interaction_error", fail.getStatus() + ": " + fail.getStatusMessage());
      }
      getMBean().notifyInteractionOk(getHost()+':'+getPort());
      return response;
    } catch (ServerOfflineException e) {
      getMBean().notifyInteractionError(getHost()+':'+getPort(), "server offline");
      throw new DeliveryException(DeliveryException.ErrorStatus.ServiceOffline, "");
    } catch (ResponseWaitTimeoutException e) {
      getMBean().notifyInteractionError(getHost()+':'+getPort(), "response wait timeout");
      throw new DeliveryException("response_timeout");
    } catch (IOException e) {
      e.printStackTrace();
      getMBean().notifyInteractionError(getHost()+':'+getPort(), "io error");
      throw new DeliveryException("interaction_error", e, e.getMessage());
    } finally {
      lock.unlock();
      if (logger.isDebugEnabled())
        logger.debug("Dcp Connection unlocked: login=" + login);
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

  public CountMessagesPackResp send(CountMessagesPack req) throws AdminException {
    return sendPdu(req, new CountMessagesPackResp());
  }

  public CountDeliveriesNextResp send(CountDeliveriesNext req) throws AdminException {
    return sendPdu(req, new CountDeliveriesNextResp());
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
