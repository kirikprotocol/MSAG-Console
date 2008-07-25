package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;
import java.util.Date;

/**
 * User: artem
 * Date: 05.06.2008
 */

class CalendarMBean extends AbstractDynamicMBean {

  private final CalendarEngine calendarEngine;

  public CalendarMBean(CalendarEngine calendarEngine) {
    super(CalendarMBean.class, "Calendar service monitor");
    this.calendarEngine = calendarEngine;

    attributes.add(new MBeanAttributeInfo("QueueSize", "java.lang.Integer", "Size of CalendarEngine messages queue", true, false, false));
    attributes.add(new MBeanAttributeInfo("EndDate", "java.util.Date", "Active period end", true, false, false));
    attributes.add(new MBeanAttributeInfo("CurrentDate", "java.util.Date", "Current date", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorActiveCount", "java.util.Integer", "Response handler active count", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorPoolSize", "java.util.Integer", "Actual handler pool size", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorMaxPoolSize", "java.util.Integer", "Response handler max pool size", true, true, false));
    attributes.add(new MBeanAttributeInfo("ExecutorRejectedTasksCount", "java.util.Integer", "Response handler rejected tasks count", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("QueueSize"))
      return calendarEngine.getQueueSize();
    else if (attribute.equals("EndDate"))
      return calendarEngine.getEndDate();
    else if (attribute.equals("CurrentDate"))
      return new Date();
    else if (attribute.equals("ExecutorActiveCount"))
      return calendarEngine.getExecutorActiveCount();
    else if (attribute.equals("ExecutorPoolSize"))
      return calendarEngine.getExecutorPoolSize();
    else if (attribute.equals("ExecutorMaxPoolSize"))
      return calendarEngine.getExecutorMaxPoolSize();
    else if (attribute.equals("ExecutorRejectedTasksCount"))
      return calendarEngine.getExecutorRejectedTasks();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
    if (attribute.getName().equals("ExecutorMaxPoolSize")) {
      int value = (Integer)attribute.getValue();
      if (value > 1)
        calendarEngine.setExecutorMaxPoolSize(value);
    }
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
