package mobi.eyeline.util.jsf.components.dynamic_table;

import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;

/**
 * @author Artem Snopkov
 */
public class DynamicTableHandler extends ComponentHandler {

  private final TagAttribute value;
  private final TagAttribute width;

  public DynamicTableHandler(ComponentConfig config) {
    super(config);
    this.value = getRequiredAttribute("value");
    this.width = getAttribute("width");
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    DynamicTable result = new DynamicTable();
    result.setValueExpression(value.getValueExpression(ctx, DynamicTableModel.class));
    result.setModel((DynamicTableModel) value.getObject(ctx, DynamicTableModel.class));
    if (width != null)
      result.setWidth(value.getInt(ctx));
    return result;
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    DynamicTable dt = (DynamicTable) c;
    dt.setModel((DynamicTableModel) value.getObject(ctx, DynamicTableModel.class));
    nextHandler.apply(ctx, c);
  }
}
