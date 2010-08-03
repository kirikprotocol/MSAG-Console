package ru.novosoft.smsc.admin.cluster_controller;

import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.*;
import ru.novosoft.smsc.util.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * @author Artem Snopkov
 */
final class ClusterControllerClient {

  private static final Logger log = Logger.getLogger(ClusterControllerClient.class);

  private static final int RESPONSE_TIMEOUT = 5000;

  private final ClusterControllerManager manager;
  private final Map<Integer, ResponseListener> listeners = new HashMap<Integer, ResponseListener>();
  private final ReceiverTask receiverThread;

  private String onlineHost;
  private Socket socket;

  ClusterControllerClient(ClusterControllerManager manager) {
    this.manager = manager;
    this.receiverThread = new ReceiverTask();
    receiverThread.start();
  }

  private synchronized Socket connect() throws AdminException {

    for (int i = 0; i < 2; i++) {

      if (log.isDebugEnabled())
        log.debug("Connecting to Cluster Controller...");

      if (onlineHost == null) { // Определяем хост, на котором запущен СС
        String host = manager.getControllerOnlineHost();
        if (host == null)
          throw new ClusterControllerException("cluster_controller_offline");
        onlineHost = host;
      }

      int port = manager.getSettings().getListenerPort();

      if (log.isDebugEnabled())
        log.debug("Cluster controller address is " + onlineHost + ':' + port);

      if (socket == null) { // Коннектимся к указанному хосту
        try {
          socket = new Socket(onlineHost, port);
        } catch (IOException e) {
          onlineHost = null;
          if (i == 1)
            throw new ClusterControllerException("cluster_controller_offline", e);

          log.error("Connection to Cluster controlled failed. One more try...");
          continue;
        }

        if (log.isDebugEnabled())
          log.debug("Connected to Cluster Controller on " + onlineHost + ':' + port + '.');
      }
    }
    return socket;
  }

  private synchronized void disconnect() {
    if (socket != null) {
      try {
        socket.close();
      } catch (IOException e) {
      }
      socket = null;

      if (log.isDebugEnabled())
        log.debug("Disconnected from Cluster Controller.");
    }
  }

  private static byte[] encodeRequest(PDU request) {
    BufferWriter buffer = new BufferWriter();
    try {
      request.encode(buffer);
    } catch (IOException e) {
      log.error(e, e);
      return null;
    }
    return buffer.getData();
  }

  private synchronized <T extends PDU> T sendPdu(PDU request, T response) throws AdminException {
    request.assignSeqNum();

    byte[] bytes = encodeRequest(request);

    ResponseListener listener = new ResponseListener(response);

    if (response != null) {
      synchronized (listeners) {
        listeners.put(request.getSeqNum(), listener);
      }
    }

    if (log.isDebugEnabled())
      log.debug("Sending PDU: " + request);

    try {
      for (int i = 0; i < 2; i++) {
        connect();
        OutputStream os = socket.getOutputStream();
        try {
          IOUtils.writeUInt32(os, bytes.length);
          IOUtils.writeUInt32(os, request.getTag());
          IOUtils.writeUInt32(os, request.getSeqNum());
          os.write(bytes);
        } catch (IOException e) {
          disconnect();
          if (i == 1) {
            if (response != null) {
              synchronized (listeners) {
                listeners.remove(request.getSeqNum());
              }
            }
            throw e;
          }
          log.error("PDU send error: seq=" + request.getSeqNum());
        }
      }
    } catch (IOException e) {
      throw new ClusterControllerException("connection_refused", e);
    }

    if (log.isDebugEnabled())
      log.debug("PDU was sent: seq= " + request.getSeqNum());

    if (response != null) {
      T resp;
      try {
        resp = (T) listener.getResponse(RESPONSE_TIMEOUT);
      } catch (InterruptedException e) {
        throw new ClusterControllerException("request_interrupted");
      }
      if (resp == null) {
        throw new ClusterControllerException("connection_refused");
      }
      return resp;
    } else
      return null;
  }

  void shutdown() {
    log.warn("ClusterControllerClient shutdowning...");

    receiverThread.started = false;
    receiverThread.interrupt();
    try {
      receiverThread.join();
    } catch (InterruptedException e) {
    }

    log.warn("ClusterControllerClient shutdowned.");
  }

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

  public AliasAddResp send(AliasAdd req) throws AdminException {
    return sendPdu(req, new AliasAddResp());
  }

  public AliasDelResp send(AliasDel req) throws AdminException {
    return sendPdu(req, new AliasDelResp());
  }

  public LockConfigResp send(LockConfig req) throws AdminException {
    return sendPdu(req, new LockConfigResp());
  }

  public void send(UnlockConfig req) throws AdminException {
    sendPdu(req, null);
  }

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

  public MscAddResp send(MscAdd req) throws AdminException {
    return sendPdu(req, new MscAddResp());
  }

  public MscRemoveResp send(MscRemove req) throws AdminException {
    return sendPdu(req, new MscRemoveResp());
  }

  public ApplyRescheduleResp send(ApplyReschedule req) throws AdminException {
    return sendPdu(req, new ApplyRescheduleResp());
  }

  public ApplyFraudControlResp send(ApplyFraudControl req) throws AdminException {
    return sendPdu(req, new ApplyFraudControlResp());
  }

  public ApplyMapLimitsResp send(ApplyMapLimits req) throws AdminException {
    return sendPdu(req, new ApplyMapLimitsResp());
  }

  public ApplySnmpResp send(ApplySnmp req) throws AdminException {
    return sendPdu(req, new ApplySnmpResp());
  }

  public GetConfigsStateResp send(GetConfigsState req) throws AdminException {
    return sendPdu(req, new GetConfigsStateResp());
  }

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

  private class ResponseListener {
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

    public void ioerror() {
      respLatch.countDown();
    }

    public int getExpectedResponseTag() {
      return response.getTag();
    }

    public void receive(BufferReader buffer, int seq) throws IOException {
      responseEx.decode(buffer);
      if (log.isDebugEnabled())
        log.debug("PDU received: seq=" + seq + ", " + responseEx);
      response = responseEx;
      respLatch.countDown();
    }
  }

  private class ReceiverTask extends Thread {

    private boolean started;

    private ReceiverTask() {
      super("ClusterControllerClient-Receiver");
      started = true;
    }

    public void run() {
      while (started) {
        try {
          Socket s = connect();
          InputStream is = s.getInputStream();

          while (!isInterrupted()) {
            int len = IOUtils.readInt32(is);
            int tag = IOUtils.readInt32(is);
            int seq = IOUtils.readInt32(is);
            byte[] body = new byte[len - 8];
            IOUtils.readFully(is, body);

            ResponseListener listener;
            synchronized (listeners) {
              listener = listeners.get(seq);
            }

            if (listener != null && listener.getExpectedResponseTag() == tag) {
              listener.receive(new BufferReader(body), seq);
            } else {
              log.error("No listener found for PDU: tag=" + tag + ", seq=" + seq);
            }
          }
        } catch (Exception e) {
          log.error(e, e);
        }

        synchronized (listeners) {
          for (ResponseListener l : listeners.values())
            l.ioerror();

          listeners.clear();
        }

        if (!isInterrupted()) {
          try {
            Thread.sleep(1000);
          } catch (InterruptedException ignored) {
          }
        }
      }
    }
  }
}
