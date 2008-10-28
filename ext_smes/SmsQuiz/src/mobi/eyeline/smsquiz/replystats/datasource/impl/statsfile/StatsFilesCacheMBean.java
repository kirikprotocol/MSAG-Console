package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class StatsFilesCacheMBean extends AbstractDynamicMBean {

  private StatsFilesCache cache;

  public StatsFilesCacheMBean(StatsFilesCache cache) {
    super(StatsFilesCache.class,"Statistics files cache's monitor");
    this.cache = cache;

    attributes.add(new MBeanAttributeInfo("DatePattern",
        "java.util.String", "Date pattern in reply's file", true, true, false));
    attributes.add(new MBeanAttributeInfo("TimePattern",
        "java.util.String", "Time pattern in reply's file", true, true, false));
    attributes.add(new MBeanAttributeInfo("ReplyStatisticsDir",
        "java.util.String", "DIrectory of reply statistics files", true, true, false));
  }
  
  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("DatePattern"))
      return cache.getDatePattern();
    else if(attribute.equals("TimePattern"))
      return  cache.getTimePattern();
    else if(attribute.equals("ReplyStatisticsDir"))
      return  cache.getReplyStatsDir();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }
  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
    if (attribute.getName().equals("DatePattern"))
      cache.setDatePattern(attribute.getValue().toString());
    else if(attribute.getName().equals("TimePattern"))
      cache.setTimePattern(attribute.getValue().toString());
    else if(attribute.getName().equals("ReplyStatisticsDir"))
      cache.setReplyStatsDir(attribute.getValue().toString());
  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
