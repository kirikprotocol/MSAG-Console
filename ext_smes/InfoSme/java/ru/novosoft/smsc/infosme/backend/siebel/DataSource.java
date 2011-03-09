package ru.novosoft.smsc.infosme.backend.siebel;

import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 31.08.2009
 * Time: 13:48:15
 * To change this template use File | Settings | File Templates.
 */
public interface DataSource {
  /// set final states for messages, see SiebelDataProvider.updateDeliveryStates

  public void saveFinalStates(Map states) throws SiebelException;

  public void taskHasFinished(String taskName) throws SiebelException;

  public boolean hasUnfinished(String waveId) throws SiebelException;
}
