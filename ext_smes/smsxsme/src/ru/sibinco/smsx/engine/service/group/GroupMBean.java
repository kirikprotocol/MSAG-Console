package ru.sibinco.smsx.engine.service.group;


import javax.management.*;

import ru.sibinco.smsx.engine.service.group.datasource.RepliesMap;
import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.utils.DataSourceException;
import mobi.eyeline.apm.utils.jmx.mbeans.AbstractDynamicMBean;

/**
 * User: artem
 * Date: 02.12.2008
 */
class GroupMBean extends AbstractDynamicMBean {

  private final RepliesMap repliesMap;
  private final DistrListDataSource listsDS;

  protected GroupMBean(RepliesMap repliesMap, DistrListDataSource listsDS) {
    super(GroupMBean.class, "Group service monitor");

    this.repliesMap = repliesMap;
    this.listsDS = listsDS;

    attributes.add(new MBeanAttributeInfo("RepliesMapSize", "java.lang.Integer", "Size of Replies Map", true, false, false));
    attributes.add(new MBeanAttributeInfo("GroupsNumber", "java.lang.Integer", "Number of groups in storage", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("RepliesMapSize"))
      return repliesMap.size();
    else if (attribute.equals("GroupsNumber"))
      try {
        return listsDS.size();
      } catch (DataSourceException e) {
        return -1;
      }
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
  }

  public Object invoke(String actionName, Object[] params, String[] signature) throws MBeanException, ReflectionException {
    return null;
  }
}
