package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.inman.AbonentContractResult;
import ru.sibinco.smpp.ub_sme.inman.InManPDUException;
import ru.sibinco.smpp.ub_sme.inbalance.InBalanceResult;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public class RequestState {
  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(RequestState.class);

  private Message abonentRequest = null;
  private long abonentRequestTime = 0;

  private AbonentContractResult inManContractResult;
  private InBalanceResult inBalanceResult;

  private Message mgResponse = null;

  private Message abonentResponse = null;
  private long abonentResponseTime = 0;

  private String banner;
  private long bannerRequestTime = 0;
  private long bannerResponseTime = 0;
  private boolean bannerRequested = false;

  private boolean error = false;
  private boolean bannerReady = false;
  private boolean balanceReady = false;

  private boolean ussdSessionClosed = false;

  private boolean[] billingSystemQueried=new boolean[SmeEngine.BILLING_SYSTEMS_COUNT];
  private long[] billingSystemsRequestTime=new long[SmeEngine.BILLING_SYSTEMS_COUNT];
  private long[] billingSystemsResponseTime=new long[SmeEngine.BILLING_SYSTEMS_COUNT];

  private int currentBillingSystemIndex = 0;
  private int currentInBalanceIN_ISDNIndex = 0;

  private boolean closed = false;

  public RequestState(Message abonentRequest, long abonentRequestTime) {
    this.abonentRequest = abonentRequest;
    this.abonentRequestTime = abonentRequestTime;
  }

  Message getAbonentRequest() {
    return abonentRequest;
  }

  AbonentContractResult getInManContractResult(){
    return inManContractResult;
  }

  void setInManContractResult(AbonentContractResult InManResponse) {
    this.inManContractResult = InManResponse;
    billingSystemsResponseTime[SmeEngine.BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE]=System.currentTimeMillis();
  }

  byte getAbonentContractType(){
    if(inManContractResult!=null){
      try {
        return inManContractResult.getContractType();
      } catch (InManPDUException e) {
        ;
      }
    }
    return SmeEngine.CONTRACT_TYPE_UNKNOWN;
  }

  InBalanceResult getInBalanceResult() {
    return inBalanceResult;
  }

  void setInBalanceResult(InBalanceResult inBalanceResult) {
    this.inBalanceResult = inBalanceResult;
    billingSystemsResponseTime[SmeEngine.BILLING_SYSTEM_IN_BALANCE]=System.currentTimeMillis();
  }

  Message getMgResponse() {
    return mgResponse;
  }

  void setMgResponse(Message mgResponse) {
    this.mgResponse = mgResponse;
    billingSystemsResponseTime[SmeEngine.BILLING_SYSTEM_FORIS_MG]=System.currentTimeMillis();
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

  public void setBannerRequested(){
    bannerRequestTime = System.currentTimeMillis();
    bannerRequested = true;
  }

  void setBanner(String banner) {
    bannerResponseTime = System.currentTimeMillis();
    this.banner = banner;
    bannerReady=true;
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

  public int getCurrentBillingSystemIndex() {
    return currentBillingSystemIndex;
  }

  public void setCurrentBillingSystemIndex(int currentBillingSystemIndex) {
    this.currentBillingSystemIndex = currentBillingSystemIndex;
  }

  public int getCurrentInBalanceIN_ISDNIndex() {
    return currentInBalanceIN_ISDNIndex;
  }

  public void setCurrentInBalanceIN_ISDNIndex(int currentInBalanceIN_ISDNIndex) {
    this.currentInBalanceIN_ISDNIndex = currentInBalanceIN_ISDNIndex;
  }

  private final static DateFormat dateFormat=new SimpleDateFormat("HH:mm:ss.SSS");

  public String toString() {
    StringBuffer sb = new StringBuffer();
    //sb.append(abonentRequest.getSourceAddress()+" request: ");
    sb.append("Request: ");
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
        sb.append(": ");
        long delay=billingSystemsResponseTime[i]-billingSystemsRequestTime[i];
        sb.append(delay);
        sb.append(" ms");
        billingSystemDelay = billingSystemDelay + delay;
      }
    }
    long bannerEngineDelay = 0;
    if(bannerRequested){
      sb.append("; banner: ");
      bannerEngineDelay = bannerResponseTime-bannerRequestTime;
      sb.append(bannerEngineDelay);
      sb.append(" ms");
    }
    sb.append("; resp=");
    long delay=abonentResponseTime-abonentRequestTime;
    sb.append(delay);
    if(logger.isDebugEnabled()){
      sb.append(" ms; SME clean delay=");
      sb.append(delay-(Math.max(billingSystemDelay, bannerEngineDelay)));
    }
    sb.append(" ms");
    return sb.toString();
  }
}
