package ru.novosoft.smsc.web.components.data_table;

import ru.novosoft.smsc.web.components.EyelineComponent;

import javax.faces.component.UIComponentBase;
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

  public Object saveState(FacesContext context) {
    Object[] values = new Object[6];
    values[0] = super.saveState(context);
    values[1] = name;
    values[2] = title;
    values[3] = width;
    values[4] = align;
    values[5] = sortable;
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
  }
}
