package mobi.eyeline.util.jsf.components.dynamic_table;

import javax.el.ELContext;
import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.servlet.jsp.JspException;
import java.util.List;

/**
 * User: artem
 * Date: 02.08.11
 */
public class SelectColumnTag extends ColumnTag {

  private ValueExpression values;
  private ValueExpression allowEditAfterAdd;
  private ValueExpression uniqueValues;

  public void setAllowEditAfterAdd(ValueExpression allowEditAfterAdd) {
    this.allowEditAfterAdd = allowEditAfterAdd;
  }

  public void setUniqueValues(ValueExpression uniqueValues) {
    this.uniqueValues = uniqueValues;
  }

  public void setValues(ValueExpression values) {
    this.values = values;
  }

  public int doStartTag() throws JspException {

    FacesContext facesContext = FacesContext.getCurrentInstance();
    ELContext elContext = facesContext.getELContext();

    SelectColumn column = new SelectColumn();
    initColumn(column, elContext);

    if (allowEditAfterAdd != null)
      column.setAllowEditAfterAdd((Boolean)getExprValue(allowEditAfterAdd, elContext));

    if (values != null)
      column.setValues((List<String>)getExprValue(values, elContext));

    if (uniqueValues != null)
      column.setUniqueValues((Boolean)getExprValue(uniqueValues, elContext));

    getTable().addColumn(column);

    return SKIP_BODY;
  }

  @Override
  public void release() {
    super.release();
    values = null;
    allowEditAfterAdd = null;
    uniqueValues = null;
  }
}
