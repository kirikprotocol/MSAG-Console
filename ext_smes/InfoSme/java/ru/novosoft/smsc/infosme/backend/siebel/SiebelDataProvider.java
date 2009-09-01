package ru.novosoft.smsc.infosme.backend.siebel;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.util.Date;
import java.util.Map;

/**
 * author: alkhal
 */
public interface SiebelDataProvider {

  public SiebelMessage getMessage(String clcId) throws SiebelDataProviderException;
  public ResultSet getMessages(String waveId) throws SiebelDataProviderException;

  public void setMessageState(String clcId, SiebelMessage.State state) throws SiebelDataProviderException;
  public SiebelMessage.State getMessageState(String clcId) throws SiebelDataProviderException;

  public void setMessageSmppState(String clcId, SiebelMessage.SmppState smppState) throws SiebelDataProviderException;
  public SiebelMessage.SmppState getMessageSmppState(String clcId) throws SiebelDataProviderException;

  public void updateDeliveryStates(Map deliveryStates) throws SiebelDataProviderException;

  public SiebelTask getTask(String waveId) throws SiebelDataProviderException;
  public ResultSet getTasks(Date fromUpdate) throws SiebelDataProviderException;
  public ResultSet getTasks() throws SiebelDataProviderException;

  public void setTaskStatus(String waveId, SiebelTask.Status status) throws SiebelDataProviderException;
  public SiebelTask.Status getTaskStatus(String waveId) throws SiebelDataProviderException;

  public void shutdown();

}