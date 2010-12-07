package mobi.eyeline.informer.web.components.set;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Artem Snopkov
 */
public class SetHandler  extends ComponentHandler {

  private final TagAttribute var;
  private final TagAttribute value;

  public SetHandler(ComponentConfig config) {
    super(config);

    var = getRequiredAttribute("var");
    value = getRequiredAttribute("value");
  }

   protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new Set();
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    Set set = (Set) c;
    set.setVar(var.getValue());
    set.setValue(value.getValueExpression(ctx, Object.class), ctx.getFacesContext().getELContext());

    ctx.getVariableMapper().setVariable(set.getVar(), set.getValue());

    nextHandler.apply(ctx, c);
  }
}
