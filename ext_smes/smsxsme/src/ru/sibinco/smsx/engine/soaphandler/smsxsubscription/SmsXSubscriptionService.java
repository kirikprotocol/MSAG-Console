package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

public interface SmsXSubscriptionService extends javax.xml.rpc.Service {
  public java.lang.String getSmsXSubscriptionAddress();

  public SmsXSubscription getSmsXSubscription() throws javax.xml.rpc.ServiceException;

  public SmsXSubscription getSmsXSubscription(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
