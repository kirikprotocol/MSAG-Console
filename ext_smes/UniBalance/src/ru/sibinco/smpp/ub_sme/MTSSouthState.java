package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;
import ru.sibinco.smpp.ub_sme.mg.MGManager;
import ru.sibinco.util.threads.ThreadsPool;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

public class MTSSouthState extends AbstractState implements BannerState, MGState {
  private final static Category logger = Category.getInstance(MTSSouthState.class);

  protected int mgState;
  protected String mgBalance;
  protected long mgResponseTime;

  protected String banner;
  protected int bannerState;
  protected long bannerResponseTime;

  protected MGManager mgManager;
  protected BannerManager bannerManager;

  public MTSSouthState(Message message, MGManager mgManager, BannerManager bannerManager) {
    super(message);
    
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
    if(mgState==MGState.MG_OK || mgState == MG_ERROR){
      mgResponseTime = System.currentTimeMillis();
    }
  }

  public String getBanner() {
    return banner;
  }

  public void setBanner(String banner) {
    this.banner = banner;
  }

  public synchronized int getBannerState() {
    return bannerState;
  }

  public synchronized void setBannerState(int bannerState) {
    this.bannerState = bannerState;
    if(bannerState==BannerState.BE_OK || bannerState==BannerState.BE_ERROR){
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

  public void closeProcessing(ThreadsPool pool, SmeEngine smeEngine) {
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

    if (mgState == MG_OK && (bannerState == BE_ERROR || bannerState == BE_OK)) {
      String message = smeEngine.prepareBalanceMessage(getMGBalance(), getBanner(), encoding);
      setMessage(message);
      smeEngine.sendResponse(this);
      close();
    }
    if (mgState == MG_ERROR) {
      smeEngine.sendErrorSmsMessage(this);
      close();
    }

  }

  public String toString() {
    StringBuffer sb = new StringBuffer();
    sb.append(abonentRequest.getSourceAddress());
    sb.append(" request: ");
    sb.append(formatDate(abonentRequestTime));
    sb.append("; mg: ");
    sb.append(mgState);
    if(mgResponseTime>0){
      sb.append(" (");
      sb.append(mgResponseTime-abonentRequestTime);
      sb.append(" ms)");
    }
    sb.append("; banner: ");
    sb.append(bannerState);
    if(bannerResponseTime>0){
      sb.append(" (");
      sb.append(bannerResponseTime-abonentRequestTime);      
      sb.append(" ms)");
    }
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
