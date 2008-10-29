package mobi.eyeline.smsquiz.distribution.impl;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class DistributionManagerMBean extends AbstractDynamicMBean {

  private DistributionInfoSmeManager manager;

  public DistributionManagerMBean(DistributionInfoSmeManager manager) {
    super(DistributionManagerMBean.class, "distributionManagerMBean");
    this.manager = manager;

    attributes.add(new MBeanAttributeInfo("DirPattern",
        "java.lang.String", "Pattern of infosme stats dir", true, false, false));
    attributes.add(new MBeanAttributeInfo("FilePattern",
        "java.lang.String", "Pattern of infosme stats file", true, false, false));
    attributes.add(new MBeanAttributeInfo("StatsDir",
        "java.lang.String", "Infosme stats directory", true, false, false));
    attributes.add(new MBeanAttributeInfo("CheckerFirstDelay",
        "java.lang.Long", "Time to delay the first execution of StatusChecker", true, false, false));
    attributes.add(new MBeanAttributeInfo("CheckerPeriod",
        "java.lang.Long", "Period of StatusChecker's execution", true, false, false));
    attributes.add(new MBeanAttributeInfo("Login",
        "java.lang.String", "Console User's name ", true, false, false));
    attributes.add(new MBeanAttributeInfo("Password",
        "java.lang.String", "Console User's password ", true, false, false));
    attributes.add(new MBeanAttributeInfo("Host",
        "java.lang.String", "Console's host", true, false, false));
    attributes.add(new MBeanAttributeInfo("Port",
        "java.lang.Integer", "Console's port", true, false, false));
    attributes.add(new MBeanAttributeInfo("CodeOk",
        "java.lang.String", "Success code of communication with console", true, false, false));
    attributes.add(new MBeanAttributeInfo("UngeneratedDistributions",
        "java.lang.String", "Get ungenerated distributions", true, false, false));
    attributes.add(new MBeanAttributeInfo("CountUngeneratedDistributions",
        "java.lang.Integer", "Count ungenerated distributions", true, false, false));
    attributes.add(new MBeanAttributeInfo("ConsoleTimeout",
        "java.lang.Long", "Timeout between closing console", true, false, false));
    attributes.add(new MBeanAttributeInfo("ConsoleCloserPeriod",
        "java.lang.Long", "Period of ConsoleCloser's execution ", true, false, false));
    attributes.add(new MBeanAttributeInfo("DateInFilePattern",
        "java.lang.String", "pattern of date in stats file ", true, false, false));
    attributes.add(new MBeanAttributeInfo("SuccDeliveryStatus",
        "java.lang.String", "infosme success delivery status", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("DirPattern")) {
      return manager.getDirPattern();
    } else if (attribute.equals("FilePattern")) {
      return manager.getFilePattern();
    } else if (attribute.equals("StatsDir")) {
      return manager.getStatsDir();
    } else if (attribute.equals("CheckerFirstDelay")) {
      return manager.getCheckerFirstDelay();
    } else if (attribute.equals("CheckerPeriod")) {
      return manager.getCheckerPeriod();
    } else if (attribute.equals("Login")) {
      return manager.getLogin();
    } else if (attribute.equals("Password")) {
      return manager.getPassword();
    } else if (attribute.equals("Host")) {
      return manager.getHost();
    } else if (attribute.equals("Port")) {
      return manager.getport();
    } else if (attribute.equals("CodeOk")) {
      return manager.getCodeok();
    } else if (attribute.equals("UngeneratedDistributions")) {
      return manager.getUngeneratedDistributions();
    } else if (attribute.equals("CountUngeneratedDistributions")) {
      return manager.countUngeneratedDistributions();
    } else if (attribute.equals("ConsoleTimeout")) {
      return manager.getConsoleTimeout();
    } else if (attribute.equals("ConsoleCloserPeriod")) {
      return manager.getCloserPeriod();
    } else if (attribute.equals("DateInFilePattern")) {
      return manager.getDateInFilePattern();
    } else if (attribute.equals("SuccDeliveryStatus")) {
      return manager.getSuccDeliveryStatus();
    }
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {

  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
