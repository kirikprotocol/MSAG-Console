package ru.sibinco.smpp.ub_sme.mg;


import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smpp.ub_sme.*;
import ru.sibinco.smpp.ub_sme.util.Convertor;
import ru.sibinco.smpp.ub_sme.util.Matcher;
import ru.sibinco.util.threads.ThreadsPool;

import java.text.MessageFormat;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.Category;

/**
 * User: pasha
 * Date: 14.01.2008
 * Time: 15:47:44
 */
public class MGRequestManager implements RequestManager {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(MgRequestStateProcessor.class);
    private SmeEngine smeEngine;
    private String balanceGatewayAddress = null;
    private String mgAddress = null;
    private Map mgRequests = new HashMap();
    private Map mgAbonentRequests = Collections.synchronizedMap(new HashMap());
    private ThreadsPool threadsPool;
    private BannerManager bannerManager;
    private MessageFormat mgRequestFormat;
    private int expireTime = 5000;
    private Matcher abonentMatcher;
    public MGRequestManager(SmeEngine smeEngine, Properties config, ThreadsPool threadsPool) {
        this.smeEngine = smeEngine;
        this.threadsPool = threadsPool;
        balanceGatewayAddress = config.getProperty("unibalance.gateway.address");
        if (balanceGatewayAddress==null||balanceGatewayAddress.length() == 0) {
            throw new InitializationException("Mandatory config parameter \"unibalance.gateway.address\" is missed");
        }
        mgAddress = config.getProperty("unibalance.mg.address", "");
        if (mgAddress.length() == 0) {
            throw new InitializationException("Mandatory config parameter \"unibalance.mg.address\" is missed");
        }
        try {
            expireTime = Integer.parseInt(config.getProperty("unibalance.expire.time"));
        } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"uniblance.expire.time\": " + config.getProperty("uniblance.expire.time"));
        }
        try{
            mgRequestFormat = new MessageFormat(config.getProperty("unibalance.mg.request.format"));
        }catch (Exception e){
            throw new InitializationException("Invalid value for config parameter unibalance.mg.request.format:"+config.getProperty("unibalance.mg.request.format"));
        }
        try{
           abonentMatcher = new Matcher(config.getProperty("unibalance.mg.abonent.pattern"));
        }catch(Exception e){
          throw new InitializationException("Invlaid value for config parameter unibalance.mg.abonent.pattern:"+config.getProperty("unibalance.mg.abonent.pattern"),e);
       }
        bannerManager = new BannerManager(config);
    }

    public void sendMgRequest(Message msg, MGState state) {
        msg.setSourceAddress(balanceGatewayAddress);
        msg.setDestinationAddress(mgAddress);
        try {
            smeEngine.getMultiplexor().assingSequenceNumber(msg, state.getAbonentRequest().getConnectionName());
            addMgRequest(msg, state);
            mgAbonentRequests.put(new Integer(msg.getUserMessageReference()), state);
            smeEngine.getMultiplexor().sendMessage(msg, false);
            if (logger.isDebugEnabled())
                logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; UserMessageReference:  " + msg.getUserMessageReference() + "; msg: " + msg.getMessageString());
        } catch (SMPPException e) {
            logger.error("Could not send MG request", e);
            state.setMgState(MGState.MG_ERR);
            removeMgRequest(msg);
            state.closeProcessing();
        }
    }

    public void sendWaitForSmsMessage(MGState state) {
        /*
        if (smsResponseMode) {
          return;
        }
        */
        Message message = prepareMessage(state);
        message.setMessageString(smeEngine.getWaitForSmsResponsePattern());
        smeEngine.sendMessage(message, state.getAbonentRequest().getConnectionName());
        if (smeEngine.getWaitForSmsResponses() != null) {
            smeEngine.getWaitForSmsResponses().count();
        }
    }

    public void sendErrorSmsMessage(MGState state) {
        /*
        if (smsResponseMode) {
          return;
        }
        */
        Message message = prepareMessage(state);
        message.setMessageString(smeEngine.getWaitForSmsResponsePattern());
        smeEngine.sendMessage(message, state.getAbonentRequest().getConnectionName());
        if (smeEngine.getWaitForSmsResponses() != null) {
            smeEngine.getWaitForSmsResponses().count();
        }
    }

    public Message prepareMessage(MGState state) {
        Message message = new Message();
        message.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
        message.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
        if (state.getAbonentRequest().hasUssdServiceOp()) {
            message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
        }
        message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        message.setType(Message.TYPE_SUBMIT);
        return message;
    }

    public String prepareBalanceMessage(String balance, String banner, String encoding) {
        if (banner == null) {
            banner = "";
        }
        if (encoding == null) {
            encoding = smeEngine.getDefaultEncoding();
        }
        MessageFormat mf = new MessageFormat(smeEngine.getBalanceResponsePattern());
        String messageString = mf.format(new String[]{balance, banner});
        if ("tr".equalsIgnoreCase(encoding) ||
                "translit".equalsIgnoreCase(encoding) ||
                "trl".equalsIgnoreCase(encoding)) {
            messageString = Convertor.translit(messageString);
        }
        return messageString;
    }

    public void requestBanner(MGState state) {
        bannerManager.RequestBanner(state.getAbonentRequest().getSourceAddress(), state);
    }

    public void response(MGState state, Message msg) {
        if (state.isExpired()) {
            smeEngine.sendDeliverSmResponse(msg, Data.ESME_RSYSERR);
            state.setMgState(MGState.MG_ERR);
            state.closeProcessing();
            return;
        }
        smeEngine.sendDeliverSmResponse(msg, Data.ESME_ROK);
        state.setMgBalance(msg.getMessageString());
        state.setMgState(MGState.MG_OK);
        state.closeProcessing();
    }

    public void sendResponse(MGState state) {
        boolean sms = false;
        if (state.getMessage().length() > smeEngine.getUssdMaxLength()) {
            sendWaitForSmsMessage(state);
            sms = true;
        }
        Message msg = new Message();
        msg.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
        msg.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
        msg.setMessageString(state.getMessage());
        if (state.getAbonentRequest().hasUssdServiceOp() && !sms) {
            msg.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
        }
        msg.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        msg.setType(Message.TYPE_SUBMIT);
        /*
        if (flashSmsEnabled) {
            msg.setDestAddrSubunit(1); // for Flash SMS
        }
        */
        smeEngine.sendMessage(msg, state.getAbonentRequest().getConnectionName());

        if (smeEngine.getResponses() != null) {
            smeEngine.getResponses().count();
        }

    }

    public void processIncomingMessage(Message message, long abonentRequestTime) {
        if (message.getSourceAddress().equals(mgAddress)) { // abonent request
            if (logger.isDebugEnabled())
                logger.debug("Got request from " + message.getSourceAddress());
           MGState state = (MGState) mgAbonentRequests.remove(new Integer(message.getUserMessageReference()));
            if (state == null) {
                if (logger.isDebugEnabled())
                    logger.debug("Request rejected because state not found");
                smeEngine.sendDeliverSmResponse(message, Data.ESME_RMSGQFUL);
            } else {
                response(state, message);
            }
        } else {
            if (smeEngine.getRequests() != null) {
                smeEngine.getRequests().count();
            }
            smeEngine.sendDeliverSmResponse(message, Data.ESME_ROK);
            MGState state = new MGState(threadsPool, message, this);
            state.startProcessing();
        }
    }

    public void handleErrorPDU(PDU pdu) {
        MGState state = (MGState) mgRequests.get(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
        if (state != null && pdu.getStatus() != PDU.STATUS_CLASS_NO_ERROR) {
            mgRequests.remove(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
            state.setMgState(MGState.MG_ERR);
            state.closeProcessing();
        }
    }

    public void addMgRequest(Message msg, StateInterface state) {
        mgRequests.put(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()), state);
    }

    public void removeMgRequest(Message msg) {
        mgRequests.remove(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()));
    }

    public int getExpireTime() {
        return expireTime;
    }

    public static Category getLogger() {
        return logger;
    }

    public MessageFormat getMgRequestFormat() {
        return mgRequestFormat;
    }

    public Matcher getAbonentMatcher() {
        return abonentMatcher;
    }
}
