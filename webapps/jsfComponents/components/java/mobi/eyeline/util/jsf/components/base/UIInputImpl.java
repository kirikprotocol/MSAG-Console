package mobi.eyeline.util.jsf.components.base;

import mobi.eyeline.util.jsf.components.MessageUtils;

import javax.el.ELException;
import javax.el.ValueExpression;
import javax.faces.application.FacesMessage;
import javax.faces.component.UIInput;
import javax.faces.context.FacesContext;

/**
 * User: artem
 * Date: 26.07.11
 */
public class UIInputImpl extends UIInput {

  /**
   * Этот метод - фикс для Apache-Myfaces 1.2. В его реализации, если ValueExpression.setValue() кидает Exception, то показывается страница с ошибкой.
   * Однако в сообветствии со спецификацией JSF (http://download.oracle.com/docs/cd/E17802_01/j2ee/j2ee/javaserverfaces/1.2/docs/api/) текст Exception-а
   * должен быть просто выведен на экран!
   *
   * В стандартной реализации JSF это правило соблюдено. Однако использовать эту реализацию мы пока не можем.
   */
  @Override
  public void updateModel(FacesContext context) {
    if (context == null) {
      throw new NullPointerException();
    }

    if (!isValid() || !isLocalValueSet()) {
      return;
    }

    ValueExpression vb = getValueExpression("value");
    if (vb != null) {
      try {
        vb.setValue(context.getELContext(), getLocalValue());
        setValue(null);
        setLocalValueSet(false);
      } catch (ELException e) {
        Throwable cause = e.getCause();
        FacesMessage message = MessageUtils.getErrorMessage(context, UIInput.CONVERSION_MESSAGE_ID);
        if(cause != null) {
          message.setDetail(cause.getLocalizedMessage());
        }
        context.addMessage(getClientId(context), message);
        setValid(false);
      }
    }
  }
}
