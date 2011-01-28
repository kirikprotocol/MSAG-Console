package mobi.eyeline.informer.web.components.graph;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class LinesHandler extends ComponentHandler{

  private final TagAttribute updatePeriod;
  private final TagAttribute labels;
  private final TagAttribute width;
  private final TagAttribute height;


  public LinesHandler(ComponentConfig config) {
    super(config);
    updatePeriod = getAttribute("updatePeriod");
    labels = getRequiredAttribute("labels");
    height = getRequiredAttribute("height");
    width = getRequiredAttribute("width");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new Lines();
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    Lines g = (Lines)c;

    if(updatePeriod != null) {
      g.setUpdatePeriod(updatePeriod.getInt(ctx));
    }

    g.setLabels((List)labels.getObject(ctx));

    g.setHeight(height.getInt(ctx));
    g.setWidth(width.getInt(ctx));

    nextHandler.apply(ctx, c);
  }
}
