package ru.novosoft.smsc.infosme.backend.soap;

public interface Blacklist extends javax.xml.rpc.Service {
  public String getblacklistSoapAddress();

  public BlacklistSoap getblacklistSoap() throws javax.xml.rpc.ServiceException;

  public BlacklistSoap getblacklistSoap(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
