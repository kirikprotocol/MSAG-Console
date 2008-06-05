package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * User: artem
 * Date: 05.06.2008
 */

class SecretMBean extends AbstractDynamicMBean {

  private final MessageSender messageSender;

  protected SecretMBean(MessageSender messageSender) {
    super(SecretMBean.class, "Secret service monitor");

    this.messageSender = messageSender;
    attributes.add(new MBeanAttributeInfo("ExecutorActiveCount", "java.util.Integer", "Response handler active count", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorPoolSize", "java.util.Integer", "Actual handler pool size", true, false, false));
    attributes.add(new MBeanAttributeInfo("ExecutorMaxPoolSize", "java.util.Integer", "Response handler max pool size", true, true, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("ExecutorActiveCount"))
      return messageSender.getExecutorActiveCount();
    else if (attribute.equals("ExecutorPoolSize"))
      return messageSender.getExecutorPoolSize();
    else if (attribute.equals("ExecutorMaxPoolSize"))
      return messageSender.getExecutorMaxPoolSize();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
    if (attribute.getName().equals("ExecutorMaxPoolSize")) {
      int value = (Integer)attribute.getValue();
      if (value > 1)
        messageSender.setExecutorMaxPoolSize(value);
    }
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
