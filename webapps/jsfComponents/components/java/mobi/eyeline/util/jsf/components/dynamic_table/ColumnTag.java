package mobi.eyeline.util.jsf.components.dynamic_table;

import javax.el.ELContext;
import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.webapp.UIComponentClassicTagBase;
import javax.faces.webapp.UIComponentELTag;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.tagext.TagSupport;

/**
 * User: artem
 * Date: 02.08.11
 */
abstract class ColumnTag extends TagSupport {

  private ValueExpression name;
  private ValueExpression title;
  private ValueExpression width;

  public void setName(ValueExpression name) {
    this.name = name;
  }

  public void setTitle(ValueExpression title) {
    this.title = title;
  }

  public void setWidth(ValueExpression width) {
    this.width = width;
  }

  protected Object getExprValue(ValueExpression expr, ELContext ctx) {
    if (expr.isLiteralText())
      return expr.getExpressionString();
    return expr.getValue(ctx);
  }

  protected void initColumn(Column c, ELContext ctx) {
    if (name != null)
      c.setName((String)getExprValue(name, ctx));
    if (title != null)
      c.setTitle((String)getExprValue(title, ctx));
    if (width != null)
      c.setWidth((String)getExprValue(width, ctx));
  }

  protected DynamicTable getTable() throws JspException {
    UIComponentClassicTagBase componentTag = UIComponentELTag.getParentUIComponentClassicTagBase(pageContext);
    if (componentTag == null) {
      throw new JspException("no parent UIComponentTag found");
    }

    UIComponent component = componentTag.getComponentInstance();
    if (component == null) {
      throw new JspException("parent UIComponentTag has no UIComponent");
    }

    return (DynamicTable) component;
  }

  @Override
  public void release() {
    super.release();
    name = null;
    title = null;
    width = null;
  }
}
