package mobi.eyeline.mcaia;

import mobi.eyeline.mcaia.protocol.*;
import mobi.eyeline.mcaia.protocol.Status;
import mobi.eyeline.protogen.framework.*;
import mobi.eyeline.smpp.api.pdu.DeliverSM;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.processing.IncomingQueue;
import mobi.eyeline.smpp.api.types.*;
import mobi.eyeline.smpp.api.util.WatchDog;
import mobi.eyeline.smpp.appgw.APPGWRequestProcessor;
import mobi.eyeline.smpp.appgw.scenario.ExecutingException;
import mobi.eyeline.smpp.sme.*;
import mobi.eyeline.smpp.sme.network.OutgoingObject;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashSet;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 18, 2010
 * Time: 1:59:26 PM
 */
public class MCAClient extends ClientConnection implements ExtendedRequestProcessor, StatusHandler, MessageStatusListener {
  private static Logger logger = Logger.getLogger(MCAClient.class);
  Address         serviceAddress;
  APPGWRequestProcessor appgw;
  OutgoingQueue outgoingQueue;
  HashSet<Integer> requests = new HashSet<Integer>(100);
  ReqWatchdog watchdog;

  public MCAClient() {
  }

  public void doRequest(BusyRequest req, boolean vlr) {
    Message msg = new DeliverSM();
    try {
      msg.setSourceAddress(req.getCalled());
      msg.setDestinationAddress(serviceAddress);
      msg.setAppInfo("mcaReq", req);
      msg.setAppInfo("mcaReqHandler", this);
      if( !vlr ) msg.setAppInfo(MessageStatusListener.APP_INFO_NAME, this);
      else msg.setAppInfo("ussdvlr", true);
      synchronized (requests) {
        requests.add( req.getSeqNum() );
      }
      appgw.initiateUssd(msg);
      watchdog.watch(req);
    } catch (InvalidAddressFormatException e) {
      logger.error("Invalid called address in request", e);
      respond(req, Status.InvalidCalledAddress);
    } catch (ExecutingException e) {
      logger.error("Could not process request", e);
      respond(req, Status.ExecutingError);
    }
  }
  public void handle(PDU pdu) {
    if( logger.isDebugEnabled() )logger.debug("Received "+pdu.toString());
    switch( ClientProtocolTag.valueOf(pdu.getTag())) {
      case BusyRequest:
        BusyRequest req = (BusyRequest) pdu;
        doRequest(req, false);
    }
  }

  public void handleStatus(BusyRequest req, Status status) {
    boolean contains = false;
    synchronized (requests) {
      contains = requests.remove(req.getSeqNum());
    }
    if( contains ) {
      respond(req, status);
      if( status == Status.Timedout ) {
        try {
          SubmitSM releaseReq = new SubmitSM();
          releaseReq.setSourceAddress(serviceAddress);
          releaseReq.setDestinationAddress(req.getCalled());
          releaseReq.setUssdServiceOp(UssdServiceOp.USSRelReq);
          outgoingQueue.addOutgoingObject(new OutgoingObject(releaseReq));
        } catch (InvalidAddressFormatException e) {
          logger.error("", e);
        }
      }
    }
  }

  public void statusChanged(Message message, mobi.eyeline.smpp.api.types.Status status, OutgoingQueue outgoingQueue) {
    if( status == mobi.eyeline.smpp.api.types.Status.OK ) {
      // in this case status will be set by executors
    } else if( status == mobi.eyeline.smpp.api.types.Status.MAPPE_NO_RESPONSE_FROM_PEER) {
      // try to deliver thru VLR, just example
      BusyRequest req = (BusyRequest) message.getAppInfo("mcaReq");
      doRequest(req, true);
    } else {
      BusyRequest req = (BusyRequest) message.getAppInfo("mcaReq");
      handleStatus(req, Status.Rejected); // todo - think about separate status
    }
  }

  protected void respond( BusyRequest req, Status status ) {
    try {
      send(new BusyResponse(req.getSeqNum(), req.getCaller(), req.getCalled(), req.getDate(), req.getCause(), req.getFlags(), status));
    } catch (IOException e) {
      logger.error("Could not respond on request seqNum="+req.getSeqNum(), e);
    }
  }

  @Override
  protected void onConnect() throws IOException {
    // no auth required, do nothing
  }

  @Override
  protected PDU onReceive(BufferReader bufferReader) throws IOException {
    return ClientProtocol.decodeMessage(bufferReader);
  }

  @Override
  protected void onSend(BufferWriter bufferWriter, PDU pdu) throws IOException {
    ClientProtocol.encodeMessage(bufferWriter, pdu);
    if( logger.isDebugEnabled() ) logger.debug("Send "+pdu);  
  }

  public void setEnvironment(IncomingQueue incomingQueue, OutgoingQueue outgoingQueue) {
    appgw.setEnvironment(incomingQueue, outgoingQueue);
    this.outgoingQueue = outgoingQueue;
  }

  @Override
  public void init(String name, Properties config) {
    watchdog = new ReqWatchdog(config, "request");
    super.init( name, config );
    try {
      serviceAddress = new Address(config.getProperty("service.address"));
    } catch (InvalidAddressFormatException e) {
      throw new InitializationException("Invalid service.address", e);
    }
    appgw = new APPGWRequestProcessor();
    appgw.init( name, config );
  }

  public void init(String name, String configFileName) throws InitializationException {
    Properties config = new Properties();
    File f = new File(configFileName);
    logger.debug("MCAClient initializing with "+f.getAbsolutePath());
    try {
      config.load(new FileReader(f));
    } catch (IOException e) {
      throw new InitializationException("Could not load mcaclient config file "+configFileName, e);
    }
    init(name, config);
  }

  public Response process(Message message) throws RequestProcessingException {
    return appgw.process(message);
  }

  public void shutdwon() {
    watchdog.shutdown();
    appgw.shutdwon();
  }

  class ReqWatchdog extends WatchDog<BusyRequest> {
    ReqWatchdog(Properties config, String prefix) {
      super(config, prefix);
    }

    @Override
    protected void expire(BusyRequest busyRequest) {
      handleStatus(busyRequest, Status.Timedout);
    }
  }
}
