package mobi.eyeline.informer.web.components.data_table;

import javax.el.ELContext;
import javax.el.ValueExpression;

/**
* @author Artem Snopkov
*/
class ConstantExpression extends ValueExpression {

  Object value;

  ConstantExpression(Object value) {
    this.value = value;
  }

  @Override
  public Object getValue(ELContext elContext) {
    return value;
  }

  @Override
  public void setValue(ELContext elContext, Object o) {
    this.value = o;
  }

  @Override
  public boolean isReadOnly(ELContext elContext) {
    return true;
  }

  @Override
  public Class<?> getType(ELContext elContext) {
    return Object.class;
  }

  @Override
  public Class<?> getExpectedType() {
    return Object.class;  //To change body of implemented methods use File | Settings | File Templates.
  }

  @Override
  public String getExpressionString() {
    return "v";
  }

  @Override
  public boolean equals(Object o) {
    return false;
  }

  @Override
  public int hashCode() {
    return 0;  //To change body of implemented methods use File | Settings | File Templates.
  }

  @Override
  public boolean isLiteralText() {
    return false;
  }
}
