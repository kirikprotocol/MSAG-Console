package mobi.eyeline.informer.web.components.input_time;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.components.collapsing_group.CollapsingGroup;

import javax.el.ELException;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;

/**
 * User: artem
 * Date: 17.01.11
 */
public class InputTimeHandler extends ComponentHandler{

  private final TagAttribute value;
  private final TagAttribute errorMessage;

  public InputTimeHandler(ComponentConfig config) {
    super(config);
    value = getRequiredAttribute("value");
    errorMessage = getAttribute("errorMessage");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    InputTime time = new InputTime();
    time.setValue((Time)value.getObject(ctx));
    time.setValueExpression(value.getValueExpression(ctx, Time.class));
    if (errorMessage != null)
      time.setErrorMessage(errorMessage.getValue(ctx));
    return time;
  }

}
