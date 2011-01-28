package mobi.eyeline.informer.web.components.graph;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * @author Aleksandr Khalitov
 */
public class LineChartHandler extends ComponentHandler{

  private final TagAttribute values;
  private final TagAttribute color;
  private final TagAttribute shade;

  public LineChartHandler(ComponentConfig config) {
    super(config);
    values = getRequiredAttribute("values");
    color = getRequiredAttribute("color");
    shade = getAttribute("shade");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new LineChart();
  }

  protected void applyNextHandler(FaceletContext ctx, UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    LineChart lineChart = (LineChart) c;
    lineChart.setValues((int[][]) values.getObject(ctx));
    lineChart.setColor((String) color.getObject(ctx));
    if(shade != null) {
      lineChart.setShade(shade.getBoolean(ctx));
    }
    nextHandler.apply(ctx, c);
  }
}
