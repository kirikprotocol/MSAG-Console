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

  private static final Logger log = Logger.getLogger(DcpClient.class);

  public static final int RESPONSE_TIMEOUT = 30000;

  private final Lock lock = new ReentrantLock();
  private final String login;
  private final String password;

  public DcpClient(String host, int port, String login, String password) {
    super(host, port, RESPONSE_TIMEOUT);
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
      if (log.isDebugEnabled())
        log.debug("Dcp Connection locked: login=" + login);

      PDU resp;
      resp = request(request, response, fail);
      
      if (resp == fail) {
        if(fail.hasStatus()) {
          String message = fail.hasStatusMessage() ?  fail.getStatusMessage() : "";
          DeliveryException.ErrorStatus e = DeliveryException.ErrorStatus.valueOf(fail.getStatus());
          if(e == null) {
            log.warn("Unknown response status="+fail.getStatus());
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
      if (log.isDebugEnabled())
        log.debug("Dcp Connection unlocked: login=" + login);
    }
  }

  public AddDeliveryMessagesResp send(AddDeliveryMessages req) throws AdminException {
    return sendPdu(req, new AddDeliveryMessagesResp());
  }

  void send(UserAuth req) throws AdminException {
    sendPdu(req, new OkResponse());
  }

}
