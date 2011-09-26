package mobi.eyeline.util.jsf.components.data_table;

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
  private String columnClass;
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

  public String getColumnClass() {
    ValueExpression exp = getValueExpression("columnClass");
    if (exp == null)
      return columnClass;
    else
      return (String) exp.getValue(getFacesContext().getELContext());
  }

  public void setColumnClass(String cls) {
    this.columnClass = cls;
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
    Object[] values = new Object[6];
    values[0] = super.saveState(context);
    values[1] = name;
    values[2] = title;
    values[3] = columnClass;
    values[4] = sortable;
    values[5] = defaultSortOrder;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    name = (String) values[1];
    title = (String) values[2];
    columnClass = (String) values[3];
    sortable = (Boolean) values[4];
    defaultSortOrder = (String) values[5];
  }
}
