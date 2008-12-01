package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;


/**
 * author: alkhal
 */
public class QuizManagerMBean extends AbstractDynamicMBean {

  private QuizManager manager;

  public QuizManagerMBean(QuizManager manager) {
    super(QuizManagerMBean.class, "quizManagerMBean");
    this.manager = manager;
    attributes.add(new MBeanAttributeInfo("CountActiveQuizes",
        "java.lang.Integer", "Count active quizes", true, false, false));
    attributes.add(new MBeanAttributeInfo("ActiveQuizes",
        "java.lang.String", "List of active quizes", true, false, false));
    attributes.add(new MBeanAttributeInfo("ResultsDir",
        "java.lang.String", "Directory with quiz results", true, false, false));
    attributes.add(new MBeanAttributeInfo("QuizDir",
        "java.lang.String", "Directory with quiz's files", true, false, false));
    attributes.add(new MBeanAttributeInfo("ListenerDelayFirst",
        "java.lang.Long", "Time to delay DirListener's execution", true, false, false));
    attributes.add(new MBeanAttributeInfo("ListenerPeriod",
        "java.lang.Long", "Period of DirListener's execution", true, false, false));
    attributes.add(new MBeanAttributeInfo("CollectorDelayFirst",
        "java.lang.Long", "Time to delay QuizCollector's execution", true, false, false));
    attributes.add(new MBeanAttributeInfo("CollectorPeriod",
        "java.lang.Long", "Period of QuizCollector's execution", true, false, false));
    attributes.add(new MBeanAttributeInfo("DirModifiedAb",
        "java.lang.String", "Directory with modified abonent's files", true, false, false));
    attributes.add(new MBeanAttributeInfo("CheckerFirstDelay",
        "java.lang.Long", "Time to delay the first execution of StatusChecker", true, false, false));
    attributes.add(new MBeanAttributeInfo("CheckerPeriod",
        "java.lang.Long", "Period of StatusChecker's execution", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("CountActiveQuizes")) {
      return manager.countQuizes();
    } else if (attribute.equals("ActiveQuizes")) {
      return manager.getAvailableQuizes();
    } else if (attribute.equals("ResultsDir")) {
      return manager.getDirResult();
    } else if (attribute.equals("QuizDir")) {
      return manager.getQuizDir();
    } else if (attribute.equals("ListenerDelayFirst")) {
      return manager.getListenerDelayFirst();
    } else if (attribute.equals("ListenerPeriod")) {
      return manager.getListenerPeriod();
    } else if (attribute.equals("CollectorDelayFirst")) {
      return manager.getCollectorDelayFirst();
    } else if (attribute.equals("CollectorPeriod")) {
      return manager.getCollectorPeriod();
    } else if (attribute.equals("DirModifiedAb")) {
      return manager.getDirWork();
    } else if (attribute.equals("CheckerFirstDelay")) {
      return manager.getCheckerFirstDelay();
    } else if (attribute.equals("CheckerPeriod")) {
      return manager.getCheckerPeriod();
    }
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {

  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
