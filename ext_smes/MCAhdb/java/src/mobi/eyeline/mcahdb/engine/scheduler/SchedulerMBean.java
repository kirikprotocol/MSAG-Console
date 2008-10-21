package mobi.eyeline.mcahdb.engine.scheduler;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

import mobi.eyeline.mcahdb.engine.scheduler.ds.TaskDataSource;
import mobi.eyeline.mcahdb.engine.scheduler.ds.TimeDataSource;
import mobi.eyeline.mcahdb.engine.DataSourceException;

/**
 * User: artem
 * Date: 21.10.2008
 */

public class SchedulerMBean extends AbstractDynamicMBean {

  private final SchedulerEngine engine;
  private final TaskDataSource ds;
  private final TimeDataSource tds;

  SchedulerMBean(SchedulerEngine engine, TaskDataSource ds, TimeDataSource tds) {
    super(SchedulerMBean.class, "Scheduler");

    this.engine = engine;
    this.ds = ds;
    this.tds = tds;

    this.attributes.add(new MBeanAttributeInfo("QueueSize", "java.lang.Integer", "Scheduler queue size", true, false, false));
    this.attributes.add(new MBeanAttributeInfo("QueueEndDate", "java.util.Date", "Scheduler end date", true, false, false));
    this.attributes.add(new MBeanAttributeInfo("TasksSize", "java.lang.Integer", "Scheduler tasks size", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("QueueSize"))
      return engine.getQueueSize();
    else if (attribute.equals("QueueEndDate"))
      try {
        return tds.getTime();
      } catch (DataSourceException e) {
        return null;
      }
    else if (attribute.equals("TasksSize"))
      return ds.size();

    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
