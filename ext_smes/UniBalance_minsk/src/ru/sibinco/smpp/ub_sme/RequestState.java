package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public class RequestState {

  private Message abonentRequest = null;
  private long abonentRequestTime = 0;

  private Message abonentResponse = null;
  private long abonentResponseTime = 0;

  private String banner;
  private long bannerRequestTime;
  private long bannerResponseTime;

  private long startCloseRequestTime;
  private long sendingResponseTime;
  private long responseSentTime;

  private boolean error = false;
  private boolean bannerReady = false;
  private boolean balanceReady = false;

  private boolean ussdSessionClosed = false;

  private boolean[] billingSystemQueried=new boolean[SmeEngine.BILLING_SYSTEMS_COUNT];
  private long[] billingSystemsRequestTime=new long[SmeEngine.BILLING_SYSTEMS_COUNT];
  private long[] billingSystemsResponseTime=new long[SmeEngine.BILLING_SYSTEMS_COUNT];

  private boolean closed = false;


  public RequestState(Message abonentRequest, long abonentRequestTime) {
    this.abonentRequest = abonentRequest;
    this.abonentRequestTime = abonentRequestTime;
  }

  Message getAbonentRequest() {
    return abonentRequest;
  }

  Message getAbonentResponse() {
    return abonentResponse;
  }

  void setAbonentResponse(Message abonentResponse) {
    this.abonentResponse = abonentResponse;
  }

  String getBanner() {
    return banner;
  }

  void setBanner(String banner) {
    this.banner = banner;
    bannerReady=true;
    bannerResponseTime = System.currentTimeMillis();
  }


  public long getBannerRequestTime() {
    return bannerRequestTime;
  }

  public void setBannerRequestTime(long bannerRequestTime) {
    this.bannerRequestTime = bannerRequestTime;
  }

  public long getAbonentRequestTime() {
    return abonentRequestTime;
  }

  public void setAbonentResponseTime(long abonentResponseTime) {
    this.abonentResponseTime = abonentResponseTime;
  }

  public boolean isError() {
    return error;
  }

  public void setError(boolean error) {
    this.error = error;
  }

  public boolean isBannerReady() {
    return bannerReady;
  }

  public boolean isBalanceReady() {
    return balanceReady;
  }

  public void setBalanceReady(boolean balanceReady) {
    this.balanceReady = balanceReady;
  }

  public boolean isUssdSessionClosed() {
    return ussdSessionClosed;
  }

  public void setUssdSessionClosed(boolean ussdSessionClosed) {
    this.ussdSessionClosed = ussdSessionClosed;
  }

  public boolean isClosed() {
    return closed;
  }

  public void setClosed(boolean closed) {
    this.closed = closed;
  }

  public boolean isBillingSystemQueried(byte systemID) {
    return billingSystemQueried[systemID];
  }

  public void setBillingSystemQueried(byte systemID) {
    billingSystemQueried[systemID]=true;
    billingSystemsRequestTime[systemID]=System.currentTimeMillis();
  }

  public void setBillingSystemResponseTime(byte systemID, long time) {
    billingSystemsResponseTime[systemID] = time;
  }

  public void setStartCloseRequestTime() {
    startCloseRequestTime = System.currentTimeMillis();
  }

  public void setSendingResponseTime() {
    sendingResponseTime = System.currentTimeMillis();
  }

  public void setResponseSentTime() {
    responseSentTime = System.currentTimeMillis();
  }

  private final static DateFormat dateFormat=new SimpleDateFormat("HH:mm:ss.SSS");

  public String toString() {
    StringBuffer sb = new StringBuffer();
    sb.append("Abonent ").append(getAbonentRequest().getSourceAddress()).append(' ');
    String date;
    synchronized(dateFormat){
      date=dateFormat.format(new Date(abonentRequestTime));
    }
    sb.append(date);
    long billingSystemDelay = 0;
    for(int i = 0; i < SmeEngine.BILLING_SYSTEMS_COUNT; i++) {
      if(billingSystemQueried[i]){
        sb.append("; ");
        sb.append(SmeEngine.BILLING_SYSTEMS[i]);
        sb.append(" response=");
        long delay=billingSystemsResponseTime[i]-billingSystemsRequestTime[i];
        sb.append(delay);
        sb.append(" ms");
        billingSystemDelay = billingSystemDelay + delay;
      }
    }
    long bannerEngineDelay = 0;
    if(bannerReady){
      sb.append("; banner engine response=");
      bannerEngineDelay = bannerResponseTime-bannerRequestTime;
      sb.append(bannerEngineDelay);
      sb.append(" ms");
    }
    sb.append("; abonent response=");
    long delay=abonentResponseTime-abonentRequestTime;
    sb.append(delay);
    sb.append(" ms; SME delay=");
    sb.append(delay-(Math.max(billingSystemDelay, bannerEngineDelay)));
    sb.append(" ms");
    sb.append(" inter times: ");
    sb.append(String.valueOf(startCloseRequestTime-abonentRequestTime)).append('/');
    sb.append(String.valueOf(sendingResponseTime-abonentRequestTime)).append('/');
    sb.append(String.valueOf(responseSentTime-abonentRequestTime));
    return sb.toString();
  }
}
