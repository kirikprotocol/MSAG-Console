package mobi.eyeline.mcahdb.engine.scheduler;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.ShutdownedException;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.InitException;
import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;
import mobi.eyeline.mcahdb.engine.scheduler.ds.TaskDataSource;
import mobi.eyeline.mcahdb.engine.scheduler.ds.TimeDataSource;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.TaskDataSourceImpl;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.TimeDataSourceImpl;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;

import javax.management.DynamicMBean;
import java.io.File;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;

/**
 * User: artem
 * Date: 27.08.2008
 */

public class Scheduler {

  private static final Category log = Category.getInstance(Scheduler.class);

  private final TaskDataSource ds;
  private final TimeDataSource tds;

  private final SchedulerEngine engine;
  private final Config config;
  private final SMPPTransceiver transceiver;
  private DynamicMBean mbean;

  public Scheduler(Config config, SMPPTransceiver transceiver) throws InitException {
    try {
      this.tds = new TimeDataSourceImpl(new File(config.getSchedulerStoreDir()));
      Date time = tds.getTime();
      if (time == null)
        time = new Date();
      tds.setTime(time);
      this.ds = new TaskDataSourceImpl(new File(config.getSchedulerStoreDir()), time);
      this.config = config;
      this.transceiver = transceiver;
      this.engine = new SchedulerEngine(config, transceiver, ds, tds);
    } catch (DataSourceException e) {
      throw new InitException(e);
    }
  }

  public void missedCall(String caller, String called, Date time) {
    try {
      Task oldTask = ds.remove(caller, called);
      if (oldTask != null)
        engine.removeTask(oldTask);      

      // Add new task
      Task t = new Task(caller, called);
      t.setTime(new Date(time.getTime() + config.getSchedulerExpirationPeriod() * 3600000));
      t.setType(Task.TYPE_EXPIRED_NOTIFY);

      ds.add(t);
      engine.addTask(t);
    } catch (DataSourceException e) {
      log.error("Can't register missed call", e);
    }
  }

  public boolean missedCallAlert(String caller, String called) {
    try {
      Task t = ds.remove(caller, called);
      if (t != null) {
        engine.removeTask(t);
        return true;
      }
    } catch (DataSourceException e) {
      log.error("Can't unregister missed call", e);
    }
    return false;
  }

  public void missedCallError(String caller, String called) {
    if (missedCallAlert(caller, called))
      sendMessage(called, caller, config.getSchedulerErrorText());
  }

  public void profileChanged(String called) {
    Collection<Task> tasks = new LinkedList<Task>();
    try {
      ds.get(called, tasks);

      for (Task t : tasks) {
        ds.remove(t);
        engine.removeTask(t);
        sendMessage(t.getCalled(), t.getCaller(), config.getSchedulerProfileChangedText());
      }

    } catch (DataSourceException e) {
      log.error("Can't unregister missed calls", e);
    }
  }

  private void sendMessage(String oa, String da, String message) {
    Message m = new Message();
    m.setSourceAddress(oa);
    m.setDestinationAddress(da);
    m.setMessageString(message);
    m.setServiceType(config.getSchedulerServiceType());
    OutgoingObject o = new OutgoingObject();
    o.setMessage(m);
    try {
      transceiver.getOutQueue().offer(o);
    } catch (ShutdownedException e) {
      log.error(e,e);
    }
  }

  public DynamicMBean getMBean() {
    if (mbean == null)
      mbean = new SchedulerMBean(engine, ds, tds);
    return mbean;
  }

  public void start() {
    engine.start();
  }

  public void shutdown() {
    engine.shutdown();
  }

  public interface Config {
    public String getSchedulerExpiredNotifText();
    public String getSchedulerErrorText();
    public String getSchedulerProfileChangedText();
    public String getSchedulerStoreDir();
    public String getSchedulerServiceType();
    public int getSchedulerExpirationPeriod();
  }
}
