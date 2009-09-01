package ru.novosoft.smsc.infosme.backend.siebel;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.util.Date;
import java.util.Collection;
import java.util.Map;

/**
 * author: alkhal
 */
public interface SiebelDataProvider {

  public SiebelMessage getMessage(String clcId) throws IntegrationDataSourceException;
  public ResultSet getMessages(String waveId) throws IntegrationDataSourceException;

  public void setMessageState(String clcId, SiebelMessage.State state) throws IntegrationDataSourceException;
  public SiebelMessage.State getMessageState(String clcId) throws IntegrationDataSourceException;

  public void setMessageSmppState(String clcId, SiebelMessage.SmppState smppState) throws IntegrationDataSourceException;
  public SiebelMessage.SmppState getMessageSmppState(String clcId) throws IntegrationDataSourceException;

  public void updateDeliveryStates(Map deliveryStates);

  public SiebelTask getTask(String waveId) throws IntegrationDataSourceException;
  public ResultSet getTasks(Date fromUpdate) throws IntegrationDataSourceException;
  public ResultSet getTasks() throws IntegrationDataSourceException;

  public void setTaskStatus(String waveId, SiebelTask.CtrlStatus status) throws IntegrationDataSourceException;
  public SiebelTask.CtrlStatus getTaskStatus(String waveId) throws IntegrationDataSourceException;

  public void shutdown();

}