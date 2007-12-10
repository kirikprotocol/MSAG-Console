package ru.sibinco.smpp.ub_sme.mg;

import com.logica.smpp.Data;
import com.lorissoft.advertising.syncclient.IAdvertisingResponseHandler;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.*;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.UnsupportedEncodingException;

/**
 * Created by Serge Lugovoy
 * Date: Nov 22, 2007
 * Time: 3:33:49 PM
 */
public class MGState implements StateInterface, IAdvertisingResponseHandler {
     private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(MGState.class);
    public static final byte MG_WAIT_RESP = 1;
    public static final byte MG_ERR = 2;
    public static final byte MG_OK = 3;
    public static final byte BE_RESP_WAIT = 4;
    public static final byte BE_RESP_ERR = 5;
    public static final byte BE_RESP_OK = 6;
    /*
    public static final byte PROF_ERR = 7;
    public static final byte PROF_OK = 8;
    public static final byte PROF_WAIT = 9;
    */
    protected int mgState;
    protected String mgBalance;
    protected String message;
    protected boolean closed = false;
    protected int bannerState;
    protected int profState;
    protected boolean expired = false;
    protected ThreadsPool pool;
    protected Message abonentRequest;
    protected long requestTime;
    protected String encoding;
    protected Object expireObject=new Object();

    public String getMgBalance() {
        return mgBalance;
    }

    public void setMgBalance(String mgBalance) {
        this.mgBalance = mgBalance;
    }

    public String getEncoding() {
        return encoding;
    }

    public void setEncoding(String encoding) {
        this.encoding = encoding;
    }

    protected String banner;

    public String getBanner() {
        return banner;
    }

    public void setBanner(String banner) {
        this.banner = banner;
    }

    public MGState(ThreadsPool pool, Message message) {
        this.pool = pool;
        this.abonentRequest = message;
        requestTime = System.currentTimeMillis();
    }


    public synchronized void expire() {
        expired = true;
    }

    public synchronized boolean isExpired() {
        return expired;
    }

    public void startProcessing() {
        pool.execute(new MgRequestStateProcessor(this));
        Sme.getSmeEngine().requestBanner(this);
        pool.execute(new ProfileStateProcessor(this));
        pool.execute(new ExpireStateProcessor(this));
    }

    public synchronized void closeProcessing() {
        if (expired || closed) return;


        if (mgState == MG_OK&&(bannerState==BE_RESP_ERR||bannerState==BE_RESP_OK)){
            String message = Sme.getSmeEngine().prepareBalanceMessage(getMgBalance(), getBanner(), getEncoding());
            setMessage(message);
            Sme.getSmeEngine().sendResponse(this);
            close();
        }
        if (mgState == MG_ERR) {
            Sme.getSmeEngine().sendErrorSmsMessage(this);
            close();
        }

    }
    public void response(Message msg){
              if (isExpired()) {
                Sme.getSmeEngine().sendDeliverSmResponse(msg, Data.ESME_RSYSERR);
            }
            setMgBalance(msg.getMessageString());
            setMgState(MGState.MG_OK);
            closeProcessing();
    }

    public Message getAbonentRequest() {
        return abonentRequest;
    }

    public synchronized int getProfState() {
        return profState;
    }

    public synchronized void setProfState(int profState) {
        this.profState = profState;
    }

    public synchronized int getMgState() {
        return mgState;
    }

    public synchronized void setMgState(int mgState) {
        this.mgState = mgState;
    }

    public synchronized boolean isClosed() {
        return closed;
    }

    public synchronized void close() {
        synchronized (expireObject){
            expireObject.notify();
        }
        this.closed = true;
    }


    public synchronized int getBannerState() {
        return bannerState;
    }

    public synchronized void setBannerState(int bannerState) {
        this.bannerState = bannerState;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Object getExpireObject() {
        return expireObject;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("request src:");
        sb.append(abonentRequest.getSourceAddress());
        sb.append(" dst:");
        sb.append(abonentRequest.getDestinationAddress());
        sb.append(" mgState:"+getMgState());
        sb.append(" bannerState:"+getBannerState());
        sb.append(" profState:"+getProfState());
        return sb.toString();
    }

    public void responseBanner(byte[] bytes) {
//                state.setBannerState(MGState.BE_RESP_WAIT);
//        String banner = Sme.getSmeEngine().getBanner(state);
        String encoding = "UTF-16BE";
        String banner = null;
        if(bytes!=null){
            try {
                banner = new String(bytes,encoding);
            } catch (UnsupportedEncodingException e) {
                logger.error("Unsupported encoding: " + encoding, e);
            }
        }
        if(logger.isDebugEnabled()){
            logger.debug("Got banner:"+banner);
        }

        if (null == banner) {
           setBannerState(MGState.BE_RESP_ERR);
        } else {
           setBanner(banner);
           setBannerState(MGState.BE_RESP_OK);
        }
        closeProcessing();
    }
}