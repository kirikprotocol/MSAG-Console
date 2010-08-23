package ru.novosoft.smsc.web.components.data_table;

import ru.novosoft.smsc.web.components.EyelineComponent;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;

import javax.el.ELContext;
import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class Row extends EyelineComponent {

  private boolean inner;
  private DataTableRow row;
  private boolean opened;
  private ConstantExpression rowIdExpr;
  private ConstantExpression dataExpr;
  private ConstantExpression innerDataExpr;

  public boolean isInner() {
    return inner;
  }

  public void setInner(boolean inner) {
    this.inner = inner;
  }

  public int getColumnsCount() {
    int res = 0;
    for (UIComponent c : getChildren()) {
      if (c instanceof Column)
        res++;
    }
    return res;
  }

  public ValueExpression getRowIdExpr() {
    if (rowIdExpr == null)
      rowIdExpr = new ConstantExpression(null);
    return rowIdExpr;
  }

  public void setRowId(ELContext ctx, String rowId) {
    getRowIdExpr().setValue(ctx, rowId);
  }

  public ValueExpression getDataExpr() {
    if (dataExpr == null)
      dataExpr = new ConstantExpression(null);
    return dataExpr;
  }

  public void setData(ELContext ctx, Object data) {
    getDataExpr().setValue(ctx, data);
  }

  public ValueExpression getInnerDataExpr() {
    if (innerDataExpr == null)
      innerDataExpr = new ConstantExpression(null);
    return innerDataExpr;
  }

  public void setInnerData(ELContext ctx, Object innerData) {
    getInnerDataExpr().setValue(ctx, innerData);
  }

  public DataTableRow getRow() {
    return row;
  }

  public void setRow(DataTableRow row) {
    this.row = row;
  }

  public boolean isOpened() {
    return opened;
  }

  public void setOpened(boolean opened) {
    this.opened = opened;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[14];
    values[0] = super.saveState(context);
    values[1] = inner;
    values[2] = rowIdExpr;
    values[3] = dataExpr;
    values[4] = innerDataExpr;
    values[5]  = row;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    inner = (Boolean) values[1];
    rowIdExpr = (ConstantExpression) values[2];
    dataExpr = (ConstantExpression) values[3];
    innerDataExpr = (ConstantExpression) values[4];
    row = (DataTableRow)values[5];
  }
}
