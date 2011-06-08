package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.EyelineComponent;

import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class Column extends EyelineComponent {

  private String name;
  private String title;
  private String width = "100%";
  private String align = "left";
  private boolean sortable;
  private String defaultSortOrder;

  public void setName(String name) {
    this.name = name;
  }

  public void setTitle(String title) {
    this.title = title;
  }

  public String getName() {
    return name;
  }

  public String getTitle() {
    return title;
  }

  public String getWidth() {
    return width;
  }

  public void setWidth(String width) {
    this.width = width;
  }

  public String getAlign() {
    return align;
  }

  public void setAlign(String align) {
    this.align = align;
  }

  public boolean isSortable() {
    return sortable;
  }

  public void setSortable(boolean sortable) {
    this.sortable = sortable;
  }

  public String getDefaultSortOrder() {
    return defaultSortOrder;
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
