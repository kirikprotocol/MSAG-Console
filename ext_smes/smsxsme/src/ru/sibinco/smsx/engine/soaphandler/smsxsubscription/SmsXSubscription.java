package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

public interface SmsXSubscription extends java.rmi.Remote {
  public CheckSubscriptionResp checkSubscription(java.lang.String msisdn) throws java.rmi.RemoteException;
}
