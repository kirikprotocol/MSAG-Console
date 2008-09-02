package mobi.eyeline.mcahdb.engine.scheduler;

import mobi.eyeline.mcahdb.engine.InitException;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;
import mobi.eyeline.mcahdb.engine.scheduler.ds.TaskDataSource;
import mobi.eyeline.mcahdb.engine.scheduler.ds.TimeDataSource;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.TimeDataSourceImpl;

import java.io.*;
import java.util.Date;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

import org.apache.log4j.Category;
import com.eyeline.utils.XDelayedQueue;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 29.08.2008
 */

class SchedulerEngine {

  private static final Category log = Category.getInstance(SchedulerEngine.class);

  private static final int TASK_FETCH_PERIOD = 3600000;

  private Date from, till;

  private final TaskDataSource ds;
  private final TimeDataSource tds;
  private final Worker worker;
  private final Scheduler.Config config;
  private final SMPPTransceiver transceiver;

  private XDelayedQueue<Task, TaskKey> queue = new XDelayedQueue<Task, TaskKey>();

  public SchedulerEngine(Scheduler.Config config, SMPPTransceiver transceiver, TaskDataSource ds, TimeDataSource tds) throws InitException {
    this.ds = ds;
    this.config = config;
    this.transceiver = transceiver;

    try {
      this.tds = tds;
      from = tds.getTime();
    } catch (DataSourceException e) {
      throw new InitException(e);
    }

    this.worker = new Worker();
  }

  public void addTask(Task t) {
    if (till != null && t.getTime().getTime() >= from.getTime() && t.getTime().getTime() <= till.getTime())
      queue.offer(t, new TaskKey(t.getCaller(), t.getCalled()), t.getTime().getTime());
  }

  public void removeTask(Task t) {
    if (till != null && t.getTime().getTime() >= from.getTime() && t.getTime().getTime() <= till.getTime())
      queue.remove(new TaskKey(t.getCaller(), t.getCalled()));
  }

  public void start() {
    worker.start();
  }

  public void shutdown() {
    worker.shutdown();
  }

  private class Worker extends Thread {

    private boolean started = true;

    private Worker() {
      super("Scheduler-Engine");
    }

    public void shutdown() {
      started = false;
    }

    public void run() {

      while (started) {
        try {
          tds.setTime(from);
        } catch (DataSourceException e) {
          log.error(e, e);
        }
        
        till = new Date(from.getTime() + TASK_FETCH_PERIOD);

        try {
          final List<Task> list = new LinkedList<Task>();
          ds.list(from, till, list);

          for (Task t : list)
            queue.offer(t, new TaskKey(t.getCaller(), t.getCalled()), t.getTime().getTime());          

          while (started || !queue.isEmpty()) {            
            Task t = queue.poll(10000);
            if (t == null)
              continue;

            Message m = new Message();
            m.setSourceAddress(t.getCalled());
            m.setDestinationAddress(t.getCaller());
            m.setMessageString(config.getSchedulerExpiredNotifText());
            OutgoingObject o = new OutgoingObject();
            o.setMessage(m);
            try {
              transceiver.getOutQueue().offer(o);
            } catch (ShutdownedException e) {
              log.error(e,e);
            }

            ds.remove(t);
          }

        } catch (DataSourceException e) {
          log.error(e,e);
        } catch (InterruptedException e) {
          log.error(e,e);
        }

        from = till;
      }
    }
  }

}
