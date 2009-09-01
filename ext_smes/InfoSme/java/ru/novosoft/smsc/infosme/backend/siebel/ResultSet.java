package ru.novosoft.smsc.infosme.backend.siebel;

public interface ResultSet {
  public boolean next() throws SiebelDataProviderException;

  public Object get() throws SiebelDataProviderException;

  public void close();
}
