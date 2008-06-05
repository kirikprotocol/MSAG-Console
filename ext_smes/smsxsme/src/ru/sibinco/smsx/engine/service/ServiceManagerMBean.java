package ru.sibinco.smsx.engine.service;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;
import java.util.Map;
import java.util.HashMap;

/**
 * User: artem
 * Date: 05.06.2008
 */

public class ServiceManagerMBean extends AbstractDynamicMBean {

  private final Map<String, ObjectName> servicesMBeans;
  private final String domain;

  protected ServiceManagerMBean(String domain) {
    super(ServiceManagerMBean.class, "Services monitor");

    this.servicesMBeans = new HashMap<String, ObjectName>(10);
    this.domain = domain;
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    return servicesMBeans.get(attribute);
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }

  @Override
  public void postRegister(Boolean registrationDone) {
    super.postRegister(registrationDone);
    try {
      { // Register calendar MBean
        Object calendarMBean = ServiceManager.getInstance().getCalendarService().getMBean(domain + ",service=calendar");
        if (calendarMBean != null) {
          attributes.add(new MBeanAttributeInfo("calendarService", ObjectName.class.getName(), "Calendar service monitor", true, false, false));

          final ObjectName name = new ObjectName(domain + ".services:name=calendar");
          getMBeanServer().registerMBean(calendarMBean, name);

          servicesMBeans.put("calendarService", name);
        }
      }

      { // Register secret MBean
        Object secretMBean = ServiceManager.getInstance().getSecretService().getMBean(domain + ",service=secret");
        if (secretMBean != null) {
          attributes.add(new MBeanAttributeInfo("secretService", ObjectName.class.getName(), "Secret service monitor", true, false, false));

          final ObjectName name = new ObjectName(domain + ".services:name=secret");
          getMBeanServer().registerMBean(secretMBean, name);

          servicesMBeans.put("secretService", name);
        }
      }

      { // Register sender MBean
        Object senderMBean = ServiceManager.getInstance().getSenderService().getMBean(domain + ",service=sender");
        if (senderMBean != null) {
          attributes.add(new MBeanAttributeInfo("senderService", ObjectName.class.getName(), "Sender service monitor", true, false, false));

          final ObjectName name = new ObjectName(domain + ".services:name=sender");
          getMBeanServer().registerMBean(senderMBean, name);

          servicesMBeans.put("senderService", name);
        }
      }

    } catch (Exception e) {
    }
  }
}
