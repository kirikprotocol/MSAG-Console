package mobi.eyeline.util.jsf.components.input_time;

import mobi.eyeline.util.jsf.components.base.UIInputImpl;

import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * @author Aleksandr Khalitov
 */
public class InputTime extends UIInputImpl implements JspTag {

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.input_time";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new InputTimeRenderer();
  }
}