package ru.sibinco.smsx.engine.service.group;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

import ru.sibinco.smsc.utils.admin.dl.DistributionListManager;

/**
 * User: artem
 * Date: 17.07.2008
 */

class GroupMBean extends AbstractDynamicMBean {

  private final GroupSendProcessor sendProcessor;
  private final DistributionListManager dlmanager;
  private final String domain;
  private DynamicMBean dlmanagerMBean;

  GroupMBean(String domain, GroupSendProcessor sendProcessor, DistributionListManager manager) {
    super(GroupMBean.class, "Group service monitor");

    this.sendProcessor = sendProcessor;
    this.dlmanager = manager;
    this.domain = domain;

    attributes.add(new MBeanAttributeInfo("ExecutorActiveCount", "java.util.Integer", "Response handler active count", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorPoolSize", "java.util.Integer", "Actual handler pool size", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorMaxPoolSize", "java.util.Integer", "Response handler max pool size", true, true, false));
    attributes.add(new MBeanAttributeInfo("ExecutorRejectedTasksCount", "java.util.Integer", "Response handler rejected tasks count", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("ExecutorActiveCount"))
      return sendProcessor.getExecutorActiveCount();
    else if (attribute.equals("ExecutorPoolSize"))
      return sendProcessor.getExecutorPoolSize();
    else if (attribute.equals("ExecutorMaxPoolSize"))
      return sendProcessor.getExecutorMaxPoolSize();
    else if (attribute.equals("ExecutorRejectedTasksCount"))
      return sendProcessor.getExecutorRejectedTasks();
    else if (attribute.equals("dlmanager"))
      return dlmanagerMBean;
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
    if (attribute.getName().equals("ExecutorMaxPoolSize")) {
      int value = (Integer)attribute.getValue();
      if (value > 1)
        sendProcessor.setExecutorMaxPoolSize(value);
    }
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }

  public void postRegister(Boolean b) {
    super.postRegister(b);

    dlmanagerMBean = dlmanager.getMBean();

    if (dlmanagerMBean != null) {
      attributes.add(new MBeanAttributeInfo("dlmanager", ObjectName.class.getName(), "Distr list manager monitor", true, false, false));

      try {
        final ObjectName name = new ObjectName(domain + ",part=dlmanager");
        getMBeanServer().registerMBean(dlmanagerMBean, name);
      } catch (Exception e) {
      }
    }
  }
}
