package ru.sibinco.smpp.ub_sme.mg;


import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smpp.ub_sme.*;
import ru.sibinco.smpp.ub_sme.util.Matcher;
import ru.sibinco.util.threads.ThreadsPool;

import java.text.MessageFormat;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

/**
 * User: pasha
 * Date: 14.01.2008
 * Time: 15:47:44
 */
public class MGManager {
  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(MGManager.class);
  private ThreadsPool threadsPool;
  private SmeEngine smeEngine;
  private String balanceGatewayAddress = null;
  private String mgAddress = null;

  private final Map statesBySN = new HashMap();
  private final Map statesByUMR = new HashMap();

  private MessageFormat mgRequestFormat;
  private Matcher abonentMatcher;

  public MGManager(ThreadsPool threadsPool, SmeEngine smeEngine) {
    this.threadsPool = threadsPool;
    this.smeEngine = smeEngine;
  }

  public void init(Properties config) throws InitializationException {
    balanceGatewayAddress = config.getProperty("sme.address");
    if (balanceGatewayAddress == null || balanceGatewayAddress.length() == 0) {
      throw new InitializationException("Mandatory config parameter \"unibalance.gateway.address\" is missed");
    }
    mgAddress = config.getProperty("mg.address", "");
    if (mgAddress.length() == 0) {
      throw new InitializationException("Mandatory config parameter \"unibalance.mg.address\" is missed");
    }
    try {
      mgRequestFormat = new MessageFormat(config.getProperty("mg.request.format"));
    } catch (Exception e) {
      throw new InitializationException("Invalid value for config parameter unibalance.mg.request.format:" + config.getProperty("mg.request.format"));
    }
    try {
      abonentMatcher = new Matcher(config.getProperty("mg.abonent.pattern"));
    } catch (Exception e) {
      throw new InitializationException("Invlaid value for config parameter unibalance.mg.abonent.pattern:" + config.getProperty("mg.abonent.pattern"), e);
    }
  }

  public void setSmeEngine(SmeEngine smeEngine) {
    this.smeEngine = smeEngine;
  }

  public void setThreadsPool(ThreadsPool threadsPool) {
    this.threadsPool = threadsPool;
  }

  public void requestMGBalance(MGState state){
    threadsPool.execute(new MGRequestThread(state));
  }

  private void sendMGRequest(Message msg, MGState state) {
    msg.setSourceAddress(balanceGatewayAddress);
    msg.setDestinationAddress(mgAddress);
    //msg.setConnectionName(state.getAbonentRequest().getConnectionName());
    try {
      smeEngine.assingSequenceNumber(msg);
      synchronized (statesBySN) {
        statesBySN.put(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()), state);
      }
      synchronized (statesByUMR) {
        statesByUMR.put(new Integer(msg.getUserMessageReference()), state);
      }
      smeEngine.sendMessage(msg, false);
      if (logger.isDebugEnabled())
        logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; UserMessageReference:  " + msg.getUserMessageReference() + "; msg: " + msg.getMessageString());
    } catch (SMPPException e) {
      logger.error("Could not send MG request", e);
      state.setMGState(MGState.MG_ERR);
      removeMGRequest(msg);
      state.closeProcessing(smeEngine);
    }
  }

  /*
    Обработка ответа MG
  */
  public void handleMGResponse(Message message) {
    MGState state;
    synchronized (statesByUMR) {
      state = (MGState) statesByUMR.remove(new Integer(message.getUserMessageReference()));
    }
    if (state == null) {
      if (logger.isDebugEnabled())
        logger.debug("MG response rejected because state not found");
      smeEngine.sendDeliverSmResponse(message, Data.ESME_RSYSERR);
    } else {
      if (state.isExpired()) {
        smeEngine.sendDeliverSmResponse(message, Data.ESME_RSYSERR);
        state.setMGState(MGState.MG_ERR);
        state.closeProcessing(threadsPool, smeEngine);
        return;
      }
      smeEngine.sendDeliverSmResponse(message, Data.ESME_ROK);
      state.setMGBalance(message.getMessageString());
      state.setMGState(MGState.MG_OK);
      state.closeProcessing(threadsPool, smeEngine);
    }
  }

  /*
    Обрабатвается PDU запроса в MG, если у PDU cтатус ошибки, то соотв. MgState закрывается.
  */
  public void handleErrorPDU(PDU pdu) {
    MGState state = (MGState) statesBySN.get(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
    if (state != null && pdu.getStatus() != PDU.STATUS_CLASS_NO_ERROR) {
      statesBySN.remove(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
      state.setMGState(MGState.MG_ERR);
      state.closeProcessing(threadsPool, smeEngine);
    }
  }

  public void removeMGRequest(Message msg) {
    statesBySN.remove(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()));
  }

  public String getMgAddress() {
    return mgAddress;
  }

  class MGRequestThread implements Runnable {

    protected MGState state;

    public MGRequestThread(MGState state) {
      this.state = state;
    }

    public void run() {
      Message message = new Message();
      message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
      String abonent = abonentMatcher.match(state.getAbonentRequest().getSourceAddress());
      String text = mgRequestFormat.format(new Object[]{abonent});
      message.setMessageString(text);
      message.setEsmClass((byte) (Data.SM_FORWARD_MODE));
      message.setType(Message.TYPE_SUBMIT);
      state.setMGState(MGState.MG_WAIT_RESP);
      sendMGRequest(message, state);
      state.closeProcessing(smeEngine);
    }


  }

}
