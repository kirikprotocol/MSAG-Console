package mobi.eyeline.util.jsf.components.data_table_filter;

import javax.el.MethodExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * author: Aleksandr Khalitov
 */
public class DataTableFilter extends UIPanel implements JspTag {

  private MethodExpression applyAction;
  private MethodExpression clearAction;

  private String action;


  public void setAction(String action) {
    this.action = action;
  }

  public void setApplyAction(MethodExpression applyAction) {
    this.applyAction = applyAction;
  }

  public void setClearAction(MethodExpression clearAction) {
    this.clearAction = clearAction;
  }

  public boolean containsApplyAction() {
    return applyAction != null;
  }

  public boolean containsClearAction() {
    return clearAction != null;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  public String getRendererType() {
    return "mobi.eyeline.util.jsf.components.data_table_filter";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new DataTableFilterRenderer();
  }


  @Override
  public void processUpdates(FacesContext context) {
    super.processUpdates(context);
    if(action != null) {
      if(action.equals("apply") && applyAction != null) {
        applyAction.invoke(context.getELContext(), new Object[0]);
      }else if (action.equals("clear") && clearAction != null) {
        clearAction.invoke(context.getELContext(), new Object[0]);
      }
    }
  }

  @Override
  public Object saveState(FacesContext context) {
    Object[] values = new Object[3];
    values[0] = super.saveState(context);
    values[1] = applyAction;
    values[2] = clearAction;
    return values;
  }

  @Override
  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[])state;
    super.restoreState(context, values[0]);
    applyAction = (MethodExpression)values[1];
    clearAction = (MethodExpression)values[2];
  }
}
