package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;
import ru.sibinco.smpp.ub_sme.mg.MGManager;
import ru.sibinco.util.threads.ThreadsPool;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;

/**
 * Created by Serge Lugovoy
 * Date: Nov 22, 2007
 * Time: 3:33:49 PM
 */
public class MTSSouthState implements BannerState, MGState {
  private final static Category logger = Category.getInstance(MTSSouthState.class);

  protected Message abonentRequest;
  protected long abonentRequestTime;

  protected boolean closed = false;
  protected boolean expired = false;

  protected String encoding;

  protected int mgState;
  protected String mgBalance;
  protected String message;
  protected long mgResponseTime;

  protected String banner;
  protected int bannerState;
  protected long bannerResponseTime;

  protected MGManager mgManager;
  protected BannerManager bannerManager;

  protected final Object expireObject = new Object();

  public MTSSouthState(Message message, MGManager mgManager, BannerManager bannerManager) {
    abonentRequestTime = System.currentTimeMillis();
    abonentRequest = message;
    if (abonentRequest.hasCodeSet()) {
      if (abonentRequest.getCodeset() == 0) {
        encoding = "tr";
      }
      if (abonentRequest.getCodeset() == 8) {
        encoding = "rus";
      }
      if (logger.isDebugEnabled()) {
        logger.debug("Set abonent message encoding:" + encoding);
      }
    }
    this.mgManager = mgManager;
    this.bannerManager = bannerManager;
  }

  public String getMGBalance() {
    return mgBalance;
  }

  public void setMGBalance(String mgBalance) {
    this.mgBalance = mgBalance;
  }

  public synchronized int getMGState() {
    return mgState;
  }

  public synchronized void setMGState(int mgState) {
    this.mgState = mgState;
    if(mgState==MGState.MG_OK || mgState == MG_ERR){
      mgResponseTime = System.currentTimeMillis();
    }
  }

  public synchronized String getBanner() {
    return banner;
  }

  public synchronized void setBanner(String banner) {
    this.banner = banner;
  }

  public synchronized int getBannerState() {
    return bannerState;
  }

  public synchronized void setBannerState(int bannerState) {
    this.bannerState = bannerState;
    if(bannerState==BannerState.BE_RESP_OK || bannerState==BannerState.BE_RESP_ERR){
      bannerResponseTime = System.currentTimeMillis();
    }
  }

  /*
    Запускается процессор запроса в MG, параллельно запрос баннера и процессор ожидания.
  */
  public void startProcessing(ThreadsPool pool, SmeEngine smeEngine) {
    mgManager.requestMGBalance(this);
    bannerManager.requestBanner(abonentRequest.getSourceAddress(), this);
    pool.execute(new ExpireStateProcessor(this, smeEngine));
  }

  public synchronized void closeProcessing(ThreadsPool pool, SmeEngine smeEngine) {
    pool.execute(new MTSSouthState.closeProcessingThread(smeEngine));
  }

  // TODO optimaze closing
  class closeProcessingThread implements Runnable {
    SmeEngine smeEngine;

    public closeProcessingThread(SmeEngine smeEngine) {
      this.smeEngine = smeEngine;
    }

    public void run() {
      closeProcessing(smeEngine);
    }
  }

  /*
    Попытка закрыть State
  */
  public synchronized void closeProcessing(SmeEngine smeEngine) {
    if (expired || closed) return;

    if (mgState == MG_OK && (bannerState == BE_RESP_ERR || bannerState == BE_RESP_OK)) {
      String message = smeEngine.prepareBalanceMessage(getMGBalance(), getBanner(), encoding);
      setMessage(message);
      smeEngine.sendResponse(this);
      close();
    }
    if (mgState == MG_ERR) {
      smeEngine.sendErrorSmsMessage(this);
      close();
    }

  }

  public Message getAbonentRequest() {
    return abonentRequest;
  }

  public synchronized void expire() {
    expired = true;
  }

  public synchronized boolean isExpired() {
    return expired;
  }

  public synchronized boolean isClosed() {
    return closed;
  }

  public synchronized void close() {
    this.closed = true;
    synchronized (expireObject) {
      expireObject.notify();
    }
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

  private final static DateFormat dateFormat=new SimpleDateFormat("HH:mm:ss.SSS");

  public String toString() {
    StringBuffer sb = new StringBuffer();
    sb.append(abonentRequest.getSourceAddress());
    sb.append(" request: ");
    String date;
    synchronized(dateFormat){
      date=dateFormat.format(new Date(abonentRequestTime));
    }
    sb.append(date);
    sb.append("; mg:");
    sb.append(mgState);
    sb.append("; banner: ");
    sb.append(bannerState);

    sb.append("; resp=");
    long delay=System.currentTimeMillis()-abonentRequestTime;
    sb.append(delay);
    if(logger.isDebugEnabled()){
      sb.append(" ms; SME clean delay=");
      sb.append(delay-(Math.max(mgResponseTime-abonentRequestTime, bannerResponseTime-abonentRequestTime)));
    }
    sb.append(" ms");
    return sb.toString();
  }

}
