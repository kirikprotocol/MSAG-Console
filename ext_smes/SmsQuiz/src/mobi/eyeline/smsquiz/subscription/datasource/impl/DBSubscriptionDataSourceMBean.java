package mobi.eyeline.smsquiz.subscription.datasource.impl;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class DBSubscriptionDataSourceMBean extends AbstractDynamicMBean {

  private DBSubscriptionDataSource ds;

  public DBSubscriptionDataSourceMBean(DBSubscriptionDataSource ds) {
    super(DBSubscriptionDataSourceMBean.class, "dbSubscriptionDataSourceMBean");
    this.ds = ds;
    attributes.add(new MBeanAttributeInfo("SQLcommands",
        "java.lang.String", "HashTable of sql commands", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("SQLcommands")) {
      return ds.sqlToString();
    }
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {

  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
