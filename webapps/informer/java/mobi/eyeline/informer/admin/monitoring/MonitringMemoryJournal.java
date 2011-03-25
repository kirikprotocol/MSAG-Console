package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.informer.admin.AdminException;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */

class MonitringMemoryJournal implements MonitoringJournal{

  private List<MonitoringEvent> events = new LinkedList<MonitoringEvent>();

  private Lock lock = new ReentrantLock();

  @Override
  public void addEvents(MonitoringEvent... events) throws AdminException {
    List<MonitoringEvent> es = new ArrayList<MonitoringEvent>(events.length); // todo На мой взгляд, сомнительная оптимизация по экономии времени под lock-ом.
    for(MonitoringEvent e : events) {                                         // todo может сразу внутри лока делать клонирование без использования вспомогательного списка?
      es.add(new MonitoringEvent(e));                                         // todo Будет проще.
    }
    try{
      lock.lock();
      this.events.addAll(es);
    }finally {
      lock.unlock();
    }
  }

  @Override
  public void visit(MonitoringFilter filter, Visitor v) throws AdminException {
    try{
      lock.lock();
      for(MonitoringEvent e : events) {
        if(filter != null && !filter.accept(e)) {
          continue;
        }
        if(!v.visit(e)) { //todo Надо клонировать ивент?
          break;
        }
      }
    }finally {
      lock.unlock();
    }
  }
}
