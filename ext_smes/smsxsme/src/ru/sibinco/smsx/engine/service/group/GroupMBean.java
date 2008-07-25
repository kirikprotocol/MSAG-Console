package ru.sibinco.smsx.engine.service.group;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * User: artem
 * Date: 17.07.2008
 */

class GroupMBean extends AbstractDynamicMBean {

  private final GroupProcessor processor;

  GroupMBean(GroupProcessor processor) {
    super(GroupMBean.class, "Group service monitor");

    this.processor = processor;

    attributes.add(new MBeanAttributeInfo("ExecutorActiveCount", "java.util.Integer", "Response handler active count", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorPoolSize", "java.util.Integer", "Actual handler pool size", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorMaxPoolSize", "java.util.Integer", "Response handler max pool size", true, true, false));
    attributes.add(new MBeanAttributeInfo("ExecutorRejectedTasksCount", "java.util.Integer", "Response handler rejected tasks count", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("ExecutorActiveCount"))
      return processor.getExecutorActiveCount();
    else if (attribute.equals("ExecutorPoolSize"))
      return processor.getExecutorPoolSize();
    else if (attribute.equals("ExecutorMaxPoolSize"))
      return processor.getExecutorMaxPoolSize();
    else if (attribute.equals("ExecutorRejectedTasksCount"))
      return processor.getExecutorRejectedTasks();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
    if (attribute.getName().equals("ExecutorMaxPoolSize")) {
      int value = (Integer)attribute.getValue();
      if (value > 1)
        processor.setExecutorMaxPoolSize(value);
    }
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
