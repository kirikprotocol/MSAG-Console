package ru.novosoft.smsc.util.smsxsender;

public interface SmsXSenderService extends javax.xml.rpc.Service {
  public java.lang.String getSmsXSenderAddress();

  public SmsXSender getSmsXSender() throws javax.xml.rpc.ServiceException;

  public SmsXSender getSmsXSender(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
