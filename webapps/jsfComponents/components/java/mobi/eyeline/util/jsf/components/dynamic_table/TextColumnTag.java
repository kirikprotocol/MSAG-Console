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
public class TextColumnTag extends ColumnTag {

  private ValueExpression allowEditAfterAdd;
  private ValueExpression maxLength;

  public void setAllowEditAfterAdd(ValueExpression allowEditAfterAdd) {
    this.allowEditAfterAdd = allowEditAfterAdd;
  }

  public void setMaxLength(ValueExpression maxLength) {
    this.maxLength = maxLength;
  }

  public int doStartTag() throws JspException {

    FacesContext facesContext = FacesContext.getCurrentInstance();
    ELContext elContext = facesContext.getELContext();

    TextColumn column = new TextColumn();
    initColumn(column, elContext);

    if (allowEditAfterAdd != null)
      column.setAllowEditAfterAdd((Boolean)getExprValue(allowEditAfterAdd, elContext));

    if (maxLength != null)
      column.setMaxLength((Integer)getExprValue(maxLength, elContext));

    getTable().addColumn(column);

    return SKIP_BODY;
  }

  @Override
  public void release() {
    super.release();
    allowEditAfterAdd = null;
    maxLength = null;
  }


}
