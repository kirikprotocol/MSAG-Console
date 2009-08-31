package ru.novosoft.smsc.infosme.backend.siebel;

public interface ResultSet {
  public boolean next() throws IntegrationDataSourceException;

  public Object get() throws IntegrationDataSourceException;

  public void close();
}
