package ru.novosoft.smsc.admin.cluster_controller;

import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.ClientConnection;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.*;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * @author Artem Snopkov
 */
final class ClusterControllerClient extends ClientConnection {

  private static final Logger log = Logger.getLogger(ClusterControllerClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;

  private final Map<Integer, ResponseListener> listeners = new HashMap<Integer, ResponseListener>();
  private boolean connected;

  ClusterControllerClient(ClusterControllerManager manager) throws AdminException {
    Properties initProps = new Properties();
    ClusterControllerSettings s = manager.getSettings();
    initProps.setProperty("cc.host", s.getListenerHost());
    initProps.setProperty("cc.port", s.getListenerPort() + "");
    init("cc", initProps);
  }

  @Override
  protected void onConnect() throws IOException {
    if (log.isDebugEnabled())
      log.debug("Connected to Cluster controller. Sending register request.");
    RegisterAsWebapp registerReq = new RegisterAsWebapp();
    registerReq.setMagic(0x57424150);
    send(registerReq);
    if (log.isDebugEnabled())
      log.debug("Register request has been sent.");
    connected = true;
  }

  @Override
  protected void onSend(BufferWriter writer, PDU pdu) throws IOException {
    int pos = writer.getLength();
    writer.writeInt(0); // write 4 bytes for future length
    writer.writeInt(pdu.getTag());
    writer.writeInt(pdu.getSeqNum());
    pdu.encode(writer);
    writer.replaceInt(pos, writer.getLength() - pos - 4);
  }

  @Override
  protected PDU onReceive(BufferReader bufferReader) throws IOException {
    int tag = bufferReader.readInt();
    int seqNum = bufferReader.readInt();

    if (log.isDebugEnabled())
      log.debug("PDU received: tag=" + tag + ", seqNum=" + seqNum);

    ResponseListener l;
    synchronized(listeners) {
      l = listeners.get(seqNum);
    }
    if (l != null) {
      if (l.getExpectedResponseTag() != tag) {
        log.error("Unexpected tag: " + tag + " for seqNum: " + seqNum);
        throw new IOException("Unexpected tag: " + tag + " for seqNum: " + seqNum);
      }

      return l.receive(bufferReader);
    } else {
      log.error("Unexpected seqNum=" + seqNum);
      throw new IOException("Unexpected seqNum=" + seqNum);
    }
  }

  @Override
  protected void handle(PDU pdu) {
  }

  private <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    ResponseListener l = new ResponseListener(response);
    int seq = request.assignSeqNum();
    if (response != null) {
      synchronized (listeners) {
        listeners.put(seq, l);
      }
    }
    try {
      if (log.isDebugEnabled())
        log.debug("Sending request: " + request);
      System.out.println("OUT: " + os);
      send(request);
      if (log.isDebugEnabled())
        log.debug("Request sent: " + request);

      if (response != null) {
        PDU resp = l.getResponse(RESPONSE_TIMEOUT);
        if (resp != null) {
          if (log.isDebugEnabled())
            log.debug("Response received: " + response);
          return (T) resp;
        } else
          throw new ClusterControllerException("response_timeout");
      }

      return null;
    } catch (InterruptedException e) {
      throw new ClusterControllerException("request_interrupted");
    } catch (IOException e) {
      connected = false;
      throw new ClusterControllerException("interaction_error", e);
    }
  }


  public boolean isConnected() {
    return connected;
  }

  // Access Controll List

  public AclAddAddressesResp send(AclAddAddresses req) throws AdminException {
    return sendPdu(req, new AclAddAddressesResp());
  }

  public AclCreateResp send(AclCreate req) throws AdminException {
    return sendPdu(req, new AclCreateResp());
  }

  public AclGetResp send(AclGet req) throws AdminException {
    return sendPdu(req, new AclGetResp());
  }

  public AclListResp send(AclList req) throws AdminException {
    return sendPdu(req, new AclListResp());
  }

  public AclLookupResp send(AclLookup req) throws AdminException {
    return sendPdu(req, new AclLookupResp());
  }

  public AclRemoveResp send(AclRemove req) throws AdminException {
    return sendPdu(req, new AclRemoveResp());
  }

  public AclRemoveAddressesResp send(AclRemoveAddresses req) throws AdminException {
    return sendPdu(req, new AclRemoveAddressesResp());
  }

  public AclUpdateResp send(AclUpdate req) throws AdminException {
    return sendPdu(req, new AclUpdateResp());
  }

  // Alias

  public AliasAddResp send(AliasAdd req) throws AdminException {
    return sendPdu(req, new AliasAddResp());
  }

  public AliasDelResp send(AliasDel req) throws AdminException {
    return sendPdu(req, new AliasDelResp());
  }

  // Config

  public LockConfigResp send(LockConfig req) throws AdminException {
    return sendPdu(req, new LockConfigResp());
  }

  public void send(UnlockConfig req) throws AdminException {
    sendPdu(req, null);
  }

  public GetConfigsStateResp send(GetConfigsState req) throws AdminException {
    return sendPdu(req, new GetConfigsStateResp());
  }

  // Closed Groups

  public CgmAddAbonentResp send(CgmAddAbonent req) throws AdminException {
    return sendPdu(req, new CgmAddAbonentResp());
  }

  public CgmAddAddrResp send(CgmAddAddr req) throws AdminException {
    return sendPdu(req, new CgmAddAddrResp());
  }

  public CgmAddGroupResp send(CgmAddGroup req) throws AdminException {
    return sendPdu(req, new CgmAddGroupResp());
  }

  public CgmCheckResp send(CgmCheck req) throws AdminException {
    return sendPdu(req, new CgmCheckResp());
  }

  public CgmDelAbonentResp send(CgmDelAbonent req) throws AdminException {
    return sendPdu(req, new CgmDelAbonentResp());
  }

  public CgmDelAddrResp send(CgmDelAddr req) throws AdminException {
    return sendPdu(req, new CgmDelAddrResp());
  }

  public CgmDeleteGroupResp send(CgmDeleteGroup req) throws AdminException {
    return sendPdu(req, new CgmDeleteGroupResp());
  }

  public CgmListAbonentsResp send(CgmListAbonents req) throws AdminException {
    return sendPdu(req, new CgmListAbonentsResp());
  }

  // Msc

  public MscAddResp send(MscAdd req) throws AdminException {
    return sendPdu(req, new MscAddResp());
  }

  public MscRemoveResp send(MscRemove req) throws AdminException {
    return sendPdu(req, new MscRemoveResp());
  }

  // Reschedule

  public ApplyRescheduleResp send(ApplyReschedule req) throws AdminException {
    return sendPdu(req, new ApplyRescheduleResp());
  }

  // Fraud

  public ApplyFraudControlResp send(ApplyFraudControl req) throws AdminException {
    return sendPdu(req, new ApplyFraudControlResp());
  }

  // Map Limits

  public ApplyMapLimitsResp send(ApplyMapLimits req) throws AdminException {
    return sendPdu(req, new ApplyMapLimitsResp());
  }

  // Snmp

  public ApplySnmpResp send(ApplySnmp req) throws AdminException {
    return sendPdu(req, new ApplySnmpResp());
  }

  // Sme

  public SmeAddResp send(SmeAdd req) throws AdminException {
    return sendPdu(req, new SmeAddResp());
  }

  public SmeRemoveResp send(SmeRemove req) throws AdminException {
    return sendPdu(req, new SmeRemoveResp());
  }

  public SmeDisconnectResp send(SmeDisconnect req) throws AdminException {
    return sendPdu(req, new SmeDisconnectResp());
  }

  public SmeStatusResp send(SmeStatus req) throws AdminException {
    return sendPdu(req, new SmeStatusResp());
  }

  public SmeUpdateResp send(SmeUpdate req) throws AdminException {
    return sendPdu(req, new SmeUpdateResp());
  }

  // Locale

  public ApplyLocaleResourceResp send(ApplyLocaleResource req) throws AdminException {
    return sendPdu(req, new ApplyLocaleResourceResp());
  }

  // Routes

  public ApplyRoutesResp send(ApplyRoutes req) throws AdminException {
    return sendPdu(req, new ApplyRoutesResp());
  }

  public TraceRouteResp send(TraceRoute req) throws AdminException {
    return sendPdu(req, new TraceRouteResp());
  }

  // Timezones

  public ApplyTimeZonesResp send(ApplyTimeZones req) throws AdminException {
    return sendPdu(req, new ApplyTimeZonesResp());
  }

  // Profiles

  public LookupProfileResp send(LookupProfile req) throws AdminException {
    return sendPdu(req, new LookupProfileResp());
  }

  public UpdateProfileResp send(UpdateProfile req) throws AdminException {
    return sendPdu(req, new UpdateProfileResp());
  }

  public DeleteProfileResp send(DeleteProfile req) throws AdminException {
    return sendPdu(req, new DeleteProfileResp());
  }

  // Loggers

  public LoggerGetCategoriesResp send(LoggerGetCategories req) throws AdminException {
    return sendPdu(req, new LoggerGetCategoriesResp());
  }

  public LoggerSetCategoriesResp send(LoggerSetCategories req) throws AdminException {
    return sendPdu(req, new LoggerSetCategoriesResp());
  }

  private static class ResponseListener {
    private final CountDownLatch respLatch = new CountDownLatch(1);

    private PDU response;
    private PDU responseEx;

    public ResponseListener(PDU responseEx) {
      this.responseEx = responseEx;
    }

    public PDU getResponse(int timeout) throws InterruptedException {
      respLatch.await(timeout, TimeUnit.MILLISECONDS);
      return response;
    }

    public int getExpectedResponseTag() {
      return responseEx.getTag();
    }

    public PDU receive(BufferReader buffer) throws IOException {
      responseEx.decode(buffer);
      response = responseEx;
      respLatch.countDown();
      return response;
    }

  }
}
