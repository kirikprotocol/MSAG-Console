package mobi.eyeline.util.jsf.components.data_table;

import javax.el.ELContext;
import javax.el.ValueExpression;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;

/**
 * @author Artem Snopkov
 */
public class Row extends UIPanel {

  private boolean inner;
  private boolean header;
  private boolean opened;
  private String rowId;
  private boolean hasInnerAttribute;
  private boolean hasInnerData;
  private boolean hasInnerRows;

  private ConstantExpression varExpr;

  private final Renderer renderer = new RowRenderer();

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return renderer;
  }

  boolean isInner() {
    return inner;
  }

  void setInner(boolean inner) {
    this.inner = inner;
  }

  String getRowId() {
    return rowId;
  }

  void setRowId(String rowId) {
    this.rowId = rowId;
  }

  public boolean hasInnerAttribute() {
    return hasInnerAttribute;
  }

  public void setHasInnerAttribute(boolean hasInnerAttribute) {
    this.hasInnerAttribute = hasInnerAttribute;
  }

  boolean hasInnerData() {
    return hasInnerData;
  }

  void setHasInnerData(boolean hasInnerData) {
    this.hasInnerData = hasInnerData;
  }

  boolean hasInnerRows() {
    return hasInnerRows;
  }

  void setHasInnerRows(boolean hasInnerRows) {
    this.hasInnerRows = hasInnerRows;
  }

  boolean isHeader() {
    return header;
  }

  void setHeader(boolean header) {
    this.header = header;
  }

  ValueExpression getVarExpr() {
    if (varExpr == null)
      varExpr = new ConstantExpression(null);
    return varExpr;
  }

  void setVar(ELContext ctx, Object value) {
    getVarExpr().setValue(ctx, value);
  }

  boolean isOpened() {
    return opened;
  }

  void setOpened(boolean opened) {
    this.opened = opened;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[3];
    values[0] = super.saveState(context);
    values[1] = inner;
    values[2] = varExpr;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    inner = (Boolean) values[1];
    varExpr = (ConstantExpression) values[2];
  }
}
