package mobi.eyeline.util.jsf.components.input_file;

import javax.faces.component.UIInput;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * author: Aleksandr Khalitov
 */
public class InputFile extends UIInput implements JspTag{

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.input_file";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new InputFileRenderer();
  }

}
