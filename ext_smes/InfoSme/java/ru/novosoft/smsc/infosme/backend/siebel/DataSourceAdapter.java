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
public class DataSourceAdapter implements DataSource {   //todo Выгоды от использования интерфейса нет.
                                                          //todo Сделать класс пакетным
                                                          //todo Нужно бросать наружу ошибки SiebelException

  private final SiebelDataProvider provider_;
  private final Category log_;

  DataSourceAdapter(SiebelDataProvider provider, Category log) {
    this.provider_ = provider;
    this.log_ = log;
  }

  public void saveFinalStates(Map states) {
    try {
      provider_.updateDeliveryStates(states);
    } catch (SiebelException e) {
      log_.error("exc to update states:" + e.getMessage());
    }
  }

  public void taskHasFinished(String waveId) {
    try {
      if (provider_.getTaskStatus(waveId) == SiebelTask.Status.IN_PROCESS) {
        provider_.setTaskStatus(waveId, SiebelTask.Status.PROCESSED);
        if (log_.isInfoEnabled()) {
          log_.info("wave " + waveId + " is processed");
        }
      }
    } catch (SiebelException e) {
      log_.error("cannot set wave " + waveId + " status: " + e.getMessage());
    }
  }

  public boolean hasUnfinished(String waveId) {
    try {
      return provider_.containsUnfinished(waveId);
    } catch (SiebelException e) {
      log_.error("Can't get unfinished messages", e);
      return true;
    }
  }
}
