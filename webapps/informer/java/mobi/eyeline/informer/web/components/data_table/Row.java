package mobi.eyeline.informer.web.components.data_table;

import mobi.eyeline.informer.web.components.EyelineComponent;

import javax.el.ELContext;
import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;

/**
 * @author Artem Snopkov
 */
public class Row extends EyelineComponent {

  private boolean inner;
//  private DataTableRow row;
  private boolean header;
  private boolean opened;
  private String rowId;
  private boolean hasInnerData;
  private boolean hasInnerRows;

  private ConstantExpression varExpr;
//  private ConstantExpression dataExpr;
//  private ConstantExpression innerDataExpr;

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

  public String getRowId() {
    return rowId;
  }

  public void setRowId(String rowId) {
    this.rowId = rowId;
  }

  public boolean hasInnerData() {
    return hasInnerData;
  }

  public void setHasInnerData(boolean hasInnerData) {
    this.hasInnerData = hasInnerData;
  }

  public boolean hasInnerRows() {
    return hasInnerRows;
  }

  public void setHasInnerRows(boolean hasInnerRows) {
    this.hasInnerRows = hasInnerRows;
  }

  public boolean isHeader() {
    return header;
  }

  public void setHeader(boolean header) {
    this.header = header;
  }

  public ValueExpression getVarExpr() {
    if (varExpr == null)
      varExpr = new ConstantExpression(null);
    return varExpr;
  }

  public void setVar(ELContext ctx, Object value) {
    getVarExpr().setValue(ctx, value);
  }
//
//  public ValueExpression getDataExpr() {
//    if (dataExpr == null)
//      dataExpr = new ConstantExpression(null);
//    return dataExpr;
//  }
//
//  public void setData(ELContext ctx, Object data) {
//    getDataExpr().setValue(ctx, data);
//  }
//
//  public ValueExpression getInnerDataExpr() {
//    if (innerDataExpr == null)
//      innerDataExpr = new ConstantExpression(null);
//    return innerDataExpr;
//  }
//
//  public void setInnerData(ELContext ctx, Object innerData) {
//    getInnerDataExpr().setValue(ctx, innerData);
//  }

//  public DataTableRow getRow() {
//    return row;
//  }
//
//  public void setRow(DataTableRow row) {
//    this.row = row;
//  }

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
    values[2] = varExpr;
//    values[3] = dataExpr;
//    values[4] = innerDataExpr;
//    values[5]  = row;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    inner = (Boolean) values[1];
    varExpr = (ConstantExpression) values[2];
//    dataExpr = (ConstantExpression) values[3];
//    innerDataExpr = (ConstantExpression) values[4];
//    row = (DataTableRow)values[5];
  }
}
