package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Category;

import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 02.09.2009
 * Time: 11:41:26
 * To change this template use File | Settings | File Templates.
 */
class DataSourceAdapter implements DataSource {

  private final SiebelDataProvider provider_;
  private final Category log_;

  DataSourceAdapter(SiebelDataProvider provider, Category log) {
    this.provider_ = provider;
    this.log_ = log;
  }

  public void saveFinalStates(Map states) throws SiebelException {
    provider_.updateDeliveryStates(states);
  }

  public void taskHasFinished(String waveId) throws SiebelException {
    if (provider_.getTaskStatus(waveId) == SiebelTask.Status.IN_PROCESS) {
      provider_.setTaskStatus(waveId, SiebelTask.Status.PROCESSED);
      if (log_.isInfoEnabled()) {
        log_.info("wave " + waveId + " is processed");
      }
    }
  }

  public boolean hasUnfinished(String waveId) throws SiebelException {
    return provider_.containsUnfinished(waveId);
  }
}
