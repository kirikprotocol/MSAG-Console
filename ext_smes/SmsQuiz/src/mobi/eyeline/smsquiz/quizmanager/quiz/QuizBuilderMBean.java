package mobi.eyeline.smsquiz.quizmanager.quiz;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class QuizBuilderMBean extends AbstractDynamicMBean {

  private QuizBuilder builder;

  public QuizBuilderMBean(QuizBuilder builder) {
    super(QuizBuilder.class, "quizBuilderMBean");
    this.builder = builder;
    attributes.add(new MBeanAttributeInfo("TimeSeparator", "java.lang.String", "Time separator in xml", true, false, false));
    attributes.add(new MBeanAttributeInfo("DatePattern", "java.lang.String", "Date pattern in xml", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("TimeSeparator")) {
      return builder.getTimeSeparator();
    } else if (attribute.equals("DatePattern")) {
      return builder.getDatePattern();
    }
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {

  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
