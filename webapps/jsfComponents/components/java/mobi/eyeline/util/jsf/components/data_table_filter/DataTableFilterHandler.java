package mobi.eyeline.util.jsf.components.data_table_filter;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;

/**
 * author: Aleksandr Khalitov
 */
public class DataTableFilterHandler extends ComponentHandler {

  private final TagAttribute apply;
  private final TagAttribute clear;


  public DataTableFilterHandler(ComponentConfig config) {
    super(config);
    apply = getAttribute("applyAction");
    clear = getAttribute("clearAction");
  }

  @Override
  protected UIComponent createComponent(FaceletContext ctx) {
    DataTableFilter filter = (DataTableFilter)super.createComponent(ctx);
    if(apply != null) {
      filter.setApplyAction(apply.getMethodExpression(ctx, Object.class, new Class[0]));
    }
    if(clear != null) {
      filter.setClearAction(clear.getMethodExpression(ctx, Object.class, new Class[0]));
    }
    return filter;
  }
}
