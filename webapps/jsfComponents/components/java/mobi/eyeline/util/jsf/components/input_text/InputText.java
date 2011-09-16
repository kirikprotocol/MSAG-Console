package mobi.eyeline.util.jsf.components.input_text;

import mobi.eyeline.util.jsf.components.base.UIInputImpl;

import javax.el.ValueExpression;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import javax.servlet.jsp.tagext.JspTag;

/**
 * author: Aleksandr Khalitov
 */
public class InputText extends UIInputImpl {

  private Integer cols;
  private Integer rows;
  private boolean secret;

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new InputTextRenderer();
  }

  public Integer getRows() {
    ValueExpression exp = getValueExpression("rows");
    if (exp == null)
      return rows;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setRows(Integer rows) {
    this.rows = rows;
  }

  public Integer getCols() {
    ValueExpression exp = getValueExpression("cols");
    if (exp == null)
      return cols;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setCols(Integer cols) {
    this.cols = cols;
  }

  public boolean isSecret() {
    ValueExpression exp = getValueExpression("secret");
    if (exp == null)
      return secret;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setSecret(boolean secret) {
    this.secret = secret;
  }

  @Override
  public Object saveState(FacesContext context) {
    Object[] values = new Object[4];
    values[0] = super.saveState(context);
    values[1] = cols;
    values[2] = rows;
    values[3] = secret;
    return values;
  }

  @Override
  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[])state;
    super.restoreState(context, values[0]);
    cols = (Integer)values[1];
    rows = (Integer)values[2];
    secret = (Boolean)values[3];
  }
}
