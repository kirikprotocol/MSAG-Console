package mobi.eyeline.informer.web.components.data_table;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class ColumnRenderer extends Renderer {


  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    Column c = (Column) component;
    Writer w = context.getResponseWriter();
    w.append("\n  <td align=\"" + c.getAlign() + "\">");
  }

  public void encodeChildren(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    if (component instanceof Column) {
      Column c = (Column) component;
      Row row = (Row)c.getParent();

      if (!row.isInner())
        super.encodeChildren(context, component);
      else {
        UIComponent f = c.getFacet("inner");
        if (f != null) {
          f.encodeBegin(context);
          f.encodeEnd(context);
        }
      }
    } else
      super.encodeChildren(context, component);
  }

  public boolean getRendersChildren() {
    return true;
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    Writer w = context.getResponseWriter();
    w.append("</td>");
  }
}
