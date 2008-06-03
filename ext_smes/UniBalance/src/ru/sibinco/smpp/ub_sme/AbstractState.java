package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;
import ru.sibinco.smpp.ub_sme.mg.MGManager;
import ru.sibinco.util.threads.ThreadsPool;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public abstract class AbstractState implements State {
  private final static Category logger = Category.getInstance(AbstractState.class);

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

  public AbstractState(Message message) {
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
  }

  public Message getAbonentRequest() {
    return abonentRequest;
  }

  public synchronized void expire() {
    expired = true;
    if(logger.isInfoEnabled())
      logger.info("Expired: "+this.toString());
  }

  public synchronized boolean isExpired() {
    return expired;
  }

  public synchronized boolean isClosed() {
    return closed;
  }

  public synchronized void close() {
    this.closed = true;
    if(logger.isInfoEnabled())
      logger.info("Closed: "+this.toString());
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

  protected static String formatDate(long tm){
    Date date = new Date(tm);
    synchronized(dateFormat){
       return dateFormat.format(date);
    }
  }

  public String toString() {
    StringBuffer sb = new StringBuffer();
    sb.append(abonentRequest.getSourceAddress());
    sb.append(" request: ");
    sb.append(formatDate(abonentRequestTime));
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
