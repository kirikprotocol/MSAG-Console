package ru.novosoft.smsc.infosme.backend.siebel;

public interface ResultSet {
  public boolean next() throws SiebelException;

  public Object get() throws SiebelException;

  public void close();
}
