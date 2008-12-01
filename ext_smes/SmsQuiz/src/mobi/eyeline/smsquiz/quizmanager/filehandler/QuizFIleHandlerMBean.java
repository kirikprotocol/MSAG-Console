package mobi.eyeline.smsquiz.quizmanager.filehandler;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

import javax.management.*;

/**
 * author: alkhal
 */
public class QuizFIleHandlerMBean extends AbstractDynamicMBean {

  private DirListener dirListener;

  public QuizFIleHandlerMBean(DirListener dirListener) {
    super(QuizFIleHandlerMBean.class, "DirListener's monitor");
    this.dirListener = dirListener;

    attributes.add(new MBeanAttributeInfo("ActiveFiles",
        "java.lang.String", "Collection of active quizes files", true, false, false));
    attributes.add(new MBeanAttributeInfo("CountActiveFiles",
        "java.lang.Integer", "Count active quizes files", true, false, false));
  }

  public Object getAttribute(String attribute) throws AttributeNotFoundException, MBeanException, ReflectionException {
    if (attribute.equals("ActiveFiles"))
      return dirListener.getFilesList();
    else if (attribute.equals("CountActiveFiles"))
      return dirListener.countFiles();
    throw new AttributeNotFoundException("Attribute " + attribute + " not found");
  }

  public void setAttribute(Attribute attribute) throws AttributeNotFoundException, InvalidAttributeValueException, MBeanException, ReflectionException {

  }

  public Object invoke(String actionName, Object params[], String signature[]) throws MBeanException, ReflectionException {
    return null;
  }
}
