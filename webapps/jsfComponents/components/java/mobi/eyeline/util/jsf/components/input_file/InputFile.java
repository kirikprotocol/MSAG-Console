package mobi.eyeline.util.jsf.components.input_file;

import javax.faces.component.UIInput;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * author: Aleksandr Khalitov
 */
public class InputFile extends UIInput {

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
