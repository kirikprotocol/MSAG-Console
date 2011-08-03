package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class Column extends UIPanel {

  private String name;
  private String title;
  private String width = "100%";
  private String align = "left";
  private boolean sortable;
  private String defaultSortOrder;

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new ColumnRenderer();
  }

  public void setName(String name) {
    this.name = name;
  }

  public void setTitle(String title) {
    this.title = title;
  }

  public String getName() {
    ValueExpression exp = getValueExpression("name");
    if (exp == null)
      return name;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public String getTitle() {
    ValueExpression exp = getValueExpression("title");
    if (exp == null)
      return title;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public String getWidth() {
    ValueExpression exp = getValueExpression("width");
    if (exp == null)
      return width;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setWidth(String width) {
    this.width = width;
  }

  public String getAlign() {
    ValueExpression exp = getValueExpression("align");
    if (exp == null)
      return align;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setAlign(String align) {
    this.align = align;
  }

  public boolean isSortable() {
    ValueExpression exp = getValueExpression("sortable");
    if (exp == null)
      return sortable;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setSortable(boolean sortable) {
    this.sortable = sortable;
  }

  public String getDefaultSortOrder() {
    ValueExpression exp = getValueExpression("defaultSortOrder");
    if (exp == null)
      return defaultSortOrder;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setDefaultSortOrder(String defaultSortOrder) {
    this.defaultSortOrder = defaultSortOrder;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[7];
    values[0] = super.saveState(context);
    values[1] = name;
    values[2] = title;
    values[3] = width;
    values[4] = align;
    values[5] = sortable;
    values[6] = defaultSortOrder;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    name = (String) values[1];
    title = (String) values[2];
    width = (String) values[3];
    align = (String) values[4];
    sortable = (Boolean) values[5];
    defaultSortOrder = (String) values[6];
  }
}
