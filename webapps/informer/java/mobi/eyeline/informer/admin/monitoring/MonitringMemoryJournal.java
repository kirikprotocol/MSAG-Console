package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.informer.admin.AdminException;

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
    try{
      lock.lock();
      for(MonitoringEvent e : events) {
        this.events.add(new MonitoringEvent(e));
      }
    }finally {
      lock.unlock();
    }
  }

  @Override
  public void visit(MonitoringEventsFilter eventsFilter, Visitor v) throws AdminException {
    try{
      lock.lock();
      for(MonitoringEvent e : events) {
        if(eventsFilter != null && !eventsFilter.accept(e)) {
          continue;
        }
        if(!v.visit(new MonitoringEvent(e))) {
          break;
        }
      }
    }finally {
      lock.unlock();
    }
  }
}
