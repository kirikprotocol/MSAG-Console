package ru.novosoft.smsc.infosme.backend.soap;

public interface BlacklistSoap extends java.rmi.Remote {
  public boolean add(java.lang.String msisdn) throws java.rmi.RemoteException;
  public boolean remove(java.lang.String msisdn) throws java.rmi.RemoteException;
}
