package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * User: artem
 * Date: 11.06.2008
 */

public class IntervalDistributionEngineMBean extends AbstractDynamicMBean {

  private final IntervalDistributionEngine engine;

  protected IntervalDistributionEngineMBean(IntervalDistributionEngine engine) {
    super(IntervalDistributionEngineMBean.class, "Distribution engine monitor");

    this.engine = engine;

    attributes.add(new MBeanAttributeInfo("deliveriesFetchTime", "java.lang.Long", "Last deliveries fetch time", true, false, false));
    attributes.add(new MBeanAttributeInfo("deliveriesQueueSize", "java.lang.Integer", "Deliveries queue size", true, false, false));
    attributes.add(new MBeanAttributeInfo("nullBannersCounter",  "java.lang.Integer", "Number of null banners returned", true, false, false));
    attributes.add(new MBeanAttributeInfo("overflows",  "java.lang.Integer", "Deliveries queue overflows", true, false, false));
    attributes.add(new MBeanAttributeInfo("threadsNumber",  "java.lang.Integer", "Number of threads", true, false, false));    
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {

    if (attribute.equals("deliveriesFetchTime"))
      return engine.getDeliveriesFetchTime();
    else if (attribute.equals("deliveriesQueueSize"))
      return engine.getDeliveriesQueueSize();
    else if (attribute.equals("nullBannersCounter"))
      return engine.getNullBannerCounter();
    else if (attribute.equals("overflows"))
      return engine.getOverflows();
    else if (attribute.equals("threadsNumber"))
      return engine.getThreadsNumber();

    return new AttributeNotFoundException(attribute);
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
