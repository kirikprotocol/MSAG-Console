package ru.novosoft.smsc.util.smsxsender;

import java.io.File;

public interface SmsXSender extends java.rmi.Remote {
  public ru.novosoft.smsc.util.smsxsender.SmsXSenderResponse sendSms(java.lang.String MSISDN, java.lang.String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC, boolean SMSXAdvertising) throws java.rmi.RemoteException;
  public ru.novosoft.smsc.util.smsxsender.SmsXSenderResponse sendPaidSms(java.lang.String sourceAddress, java.lang.String destinationAddress, java.lang.String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC) throws java.rmi.RemoteException;
  public ru.novosoft.smsc.util.smsxsender.SmsXSenderResponse checkStatus(java.lang.String SMSXIdMessage) throws java.rmi.RemoteException;
  public int batchSecret(java.lang.String sourceAddress, java.lang.String message, boolean SMSXExpress, File destinations) throws java.rmi.RemoteException;
  public int sendSysSms(java.lang.String sourceAddress, java.lang.String destinationAddress, java.lang.String message) throws java.rmi.RemoteException;
}
