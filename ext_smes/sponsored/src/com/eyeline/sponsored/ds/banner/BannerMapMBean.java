package com.eyeline.sponsored.ds.banner;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * User: artem
 * Date: 26.05.2008
 */

public class BannerMapMBean extends AbstractDynamicMBean {

  private final BannerMap bannerMap;

  public BannerMapMBean(BannerMap bannerMap) {
    super(BannerMapMBean.class, "Banner map monitor");

    this.bannerMap = bannerMap;

    this.attributes.add(new MBeanAttributeInfo("size", "java.lang.Integer", "Map size", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("size"))
      return bannerMap.size();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found in " + BannerMapMBean.class.getName());
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
