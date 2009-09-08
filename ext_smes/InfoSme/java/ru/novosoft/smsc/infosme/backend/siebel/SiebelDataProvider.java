package ru.novosoft.smsc.infosme.backend.siebel;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.util.Date;
import java.util.Map;

/**
 * author: alkhal
 */
public interface SiebelDataProvider {

  public SiebelMessage getMessage(String clcId) throws SiebelException;
  public ResultSet getMessages(String waveId) throws SiebelException;

  public void setMessageState(String clcId, SiebelMessage.State state) throws SiebelException;
  public SiebelMessage.State getMessageState(String clcId) throws SiebelException;

  /**
   * @param deliveryStates - map, where key is String with message Id, value is instance of SiebelMessage.DeliveryState
   * @throws SiebelException
   */
  public void updateDeliveryStates(Map deliveryStates) throws SiebelException;

  public SiebelTask getTask(String waveId) throws SiebelException;
 // public ResultSet getTasks(Date fromUpdate) throws SiebelException;
  public ResultSet getTasks(Date fromUpdate, Date tillUpdate) throws SiebelException;
  public ResultSet getTasks() throws SiebelException;

  public void setTaskStatus(String waveId, SiebelTask.Status status) throws SiebelException;
  public SiebelTask.Status getTaskStatus(String waveId) throws SiebelException;

  public void shutdown();

}