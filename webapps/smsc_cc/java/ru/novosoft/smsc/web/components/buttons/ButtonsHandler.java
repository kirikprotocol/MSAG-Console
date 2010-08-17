package ru.novosoft.smsc.web.components.buttons;

import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import ru.novosoft.smsc.web.components.dynamic_table.DynamicTable;

import javax.el.MethodExpression;
import javax.faces.event.ActionEvent;

/**
 * @author Artem Snopkov
 */
public class ButtonsHandler extends ComponentHandler {

  public ButtonsHandler(ComponentConfig config) {
    super(config);
  }

  protected javax.faces.component.UIComponent createComponent(com.sun.facelets.FaceletContext ctx) {
    return new Buttons();
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    Buttons b = (Buttons)c;

    if (b.getSelectedButton() != null) {
      MethodExpression m = ctx.getExpressionFactory().createMethodExpression(ctx, b.getSelectedButton(), Void.class, new Class[]{ActionEvent.class});
      if (m != null)
        m.invoke(ctx, new Object[]{new ActionEvent(b)});
    }

    nextHandler.apply(ctx, c);
  }
}
