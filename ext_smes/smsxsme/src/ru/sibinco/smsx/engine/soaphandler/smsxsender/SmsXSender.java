package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import java.rmi.RemoteException;

public interface SmsXSender extends java.rmi.Remote {
  public SmsXSenderResponse sendSms(String MSISDN, String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC, boolean SMSXAdvertising) throws java.rmi.RemoteException;
  public SmsXSenderResponse sendPaidSms(String oa, String da, String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC) throws java.rmi.RemoteException;
  public int sendSysSms(String oa, String da, String message) throws java.rmi.RemoteException;
  public SmsXSenderResponse checkStatus(java.lang.String SMSXIdMessage) throws java.rmi.RemoteException;
  public int batchSecret(String oa, String message, boolean express) throws RemoteException;
}
