package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class StatsFilesCacheMBean extends AbstractDynamicMBean {

  private StatsFilesCache cache;

  public StatsFilesCacheMBean(StatsFilesCache cache) {
    super(StatsFilesCacheMBean.class, "Statistics files cache's monitor");
    this.cache = cache;

    attributes.add(new MBeanAttributeInfo("DatePattern",
        "java.lang.String", "Date pattern in reply's file", true, false, false));
    attributes.add(new MBeanAttributeInfo("TimePattern",
        "java.lang.String", "Time pattern in reply's file", true, false, false));
    attributes.add(new MBeanAttributeInfo("ReplyStatisticsDir",
        "java.lang.String", "DIrectory of reply statistics files", true, false, false));
    attributes.add(new MBeanAttributeInfo("ReplyFileNamePattern",
        "java.lang.String", "File's name pattern", true, false, false));
    attributes.add(new MBeanAttributeInfo("CountOpenedFiles",
        "java.lang.Integer", "Count opened files", true, false, false));
    attributes.add(new MBeanAttributeInfo("GetOpenedFiles",
        "java.lang.String", "List of opened file", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("DatePattern"))
      return cache.getDatePattern();
    else if (attribute.equals("TimePattern"))
      return cache.getTimePattern();
    else if (attribute.equals("ReplyStatisticsDir"))
      return cache.getReplyStatsDir();
    else if (attribute.equals("ReplyFileNamePattern"))
      return cache.getFileNamePattern();
    else if (attribute.equals("CountOpenedFiles"))
      return cache.countOpenedFiles();
    else if (attribute.equals("GetOpenedFiles"))
      return cache.getOpenedFiles();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {

  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
