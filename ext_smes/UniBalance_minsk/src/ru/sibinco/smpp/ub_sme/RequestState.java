package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.lorissoft.advertising.client.RequestContext;

public class RequestState {

  private Message abonentRequest = null;
  private long abonentRequestTime = 0;

  private Message abonentResponse = null;
  private long abonentResponseTime = 0;

  private String banner;
  private long bannerRequestTime;
  private long bannerResponseTime;
  private RequestContext bannerRequestContext;

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

  public Message getAbonentRequest() {
    return abonentRequest;
  }

  public Message getAbonentResponse() {
    return abonentResponse;
  }

  public void setAbonentResponse(Message abonentResponse) {
    this.abonentResponse = abonentResponse;
  }

  public String getBanner() {
    return banner;
  }

  public void setBanner(String banner, long bannerResponseTime) {
    this.banner = banner;
    bannerReady=true;
    this.bannerResponseTime = bannerResponseTime;
  }

  public RequestContext getBannerRequestContext() {
    return bannerRequestContext;
  }

  public void setBannerRequestContext(RequestContext bannerRequestContext) {
    this.bannerRequestContext = bannerRequestContext;
  }

  public long getBannerRequestTime() {
    return bannerRequestTime;
  }

  public void setBannerRequestTime(long bannerRequestTime) {
    this.bannerRequestTime = bannerRequestTime;
  }

  public long getBannerResponseTime() {
    return bannerResponseTime>0 ? bannerResponseTime : System.currentTimeMillis();
  }

  public boolean isBannerQueried(){
    return bannerRequestTime>0;
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

  public long getBillingResponseDelay(){
    long delay=0;
    for(int i = 0; i < SmeEngine.BILLING_SYSTEMS_COUNT; i++) {
      if(billingSystemQueried[i]){
        delay=delay+billingSystemsResponseTime[i]-billingSystemsRequestTime[i];
      }
    }
    return delay;
  }

  private final static DateFormat dateFormat=new SimpleDateFormat("HH:mm:ss.SSS");

  public String toString() {
    StringBuffer sb = new StringBuffer();
    String date;
    synchronized(dateFormat){
      date=dateFormat.format(new Date(abonentRequestTime));
    }
    sb.append(date).append(' ').append(getAbonentRequest().getSourceAddress()).append(':').append(' ');
    long billingSystemDelay = 0;
    for(int i = 0; i < SmeEngine.BILLING_SYSTEMS_COUNT; i++) {
      if(billingSystemQueried[i]){
        sb.append(SmeEngine.BILLING_SYSTEMS[i]);
        sb.append(" response=");
        long delay=billingSystemsResponseTime[i]-billingSystemsRequestTime[i];
        sb.append(String.valueOf(delay)).append('/');
        sb.append(String.valueOf(billingSystemsRequestTime[i]-abonentRequestTime));
        sb.append(" ms; ");
        billingSystemDelay = billingSystemDelay + delay;
      }
    }
    long bannerEngineDelay = 0;
    if(bannerReady){
      sb.append("BE response=");
      bannerEngineDelay = getBannerResponseTime()-getBannerRequestTime();
      sb.append(String.valueOf(bannerEngineDelay)).append('/');
      sb.append(String.valueOf(getBannerRequestTime() - abonentRequestTime));
      sb.append(" ms; ");
    }
    sb.append("abonent response=");
    long delay=abonentResponseTime-abonentRequestTime;
    sb.append(String.valueOf(delay));
    sb.append(" ms; SME delay=");
    sb.append(String.valueOf(delay-(Math.max(billingSystemDelay, bannerEngineDelay))));
    sb.append(" ms");
    sb.append("; inter times: ");
    sb.append(String.valueOf(startCloseRequestTime-abonentRequestTime)).append('/');
    sb.append(String.valueOf(sendingResponseTime-abonentRequestTime)).append('/');
    sb.append(String.valueOf(responseSentTime-abonentRequestTime));
    return sb.toString();
  }
}
