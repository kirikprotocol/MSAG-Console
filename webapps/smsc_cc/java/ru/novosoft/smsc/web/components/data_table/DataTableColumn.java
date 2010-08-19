package ru.novosoft.smsc.web.components.data_table;

import javax.faces.component.UIComponentBase;
import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class DataTableColumn extends UIComponentBase {

  private String name;
  private String title;
  private String width = "100%";
  private String align = "left";
  private boolean sortable;
  private String var;
  private String rowNumVar;
  private int rowNum;
  private ConstantExpression varExpression;
  private ConstantExpression rowNumExpression;

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

  public String getVar() {
    return var;
  }

  public void setVar(String var) {
    this.var = var;
  }

  public int getRowNum() {
    return rowNum;
  }

  public void setRowNum(int rowNum) {
    this.rowNum = rowNum;
  }

  public ConstantExpression getVarExpression() {
    return varExpression;
  }

  public void setVarExpression(ConstantExpression varExpression) {
    this.varExpression = varExpression;
  }

  public String getRowNumVar() {
    return rowNumVar;
  }

  public void setRowNumVar(String rowNumVar) {
    this.rowNumVar = rowNumVar;
  }

  public ConstantExpression getRowNumExpression() {
    return rowNumExpression;
  }

  public void setRowNumExpression(ConstantExpression rowNumExpression) {
    this.rowNumExpression = rowNumExpression;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[12];
    values[0] = super.saveState(context);
    values[1] = name;
    values[2] = title;
    values[3] = width;
    values[4] = align;
    values[5] = rowNumVar;
    values[6] = sortable;
    values[7] = var;
    values[9] = rowNum;
    values[10] = varExpression;
    values[11] = rowNumExpression;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    name = (String) values[1];
    title = (String) values[2];
    width = (String) values[3];
    align = (String) values[4];
    rowNumVar = (String)values[5];
    sortable = (Boolean) values[6];
    var = (String) values[7];
    rowNum = (Integer) values[9];
    varExpression = (ConstantExpression)values[10];
    rowNumExpression = (ConstantExpression)values[11];
  }
}
