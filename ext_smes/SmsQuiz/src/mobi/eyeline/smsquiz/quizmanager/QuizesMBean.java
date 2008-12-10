package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class QuizesMBean extends AbstractDynamicMBean {

  private final Quizes quizes;

  public QuizesMBean(Quizes quizes) {
    super(Quizes.class, "quizesMBean");
    this.quizes = quizes;
    attributes.add(new MBeanAttributeInfo("CountQuizes",
        "java.lang.String", "Count quizes", true, false, false));
    attributes.add(new MBeanAttributeInfo("Quizes",
        "java.lang.String", "Quizes", true, false, false));

  }
  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("CountQuizes")) {
      return quizes.countQuizes();
    } else if (attribute.equals("Quizes")) {
      return quizes.getQuizesToString("**************************");
    }
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {
  }

  public Object invoke(String actionName, Object[] params, String[] signature) throws MBeanException, ReflectionException {
    return null;
  }
}
