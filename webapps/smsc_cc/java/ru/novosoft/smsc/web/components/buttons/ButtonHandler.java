package ru.novosoft.smsc.web.components.buttons;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.FaceletException;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.TagConfig;
import com.sun.facelets.tag.TagHandler;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import ru.novosoft.smsc.web.components.dynamic_table.DynamicTable;

import javax.el.ELException;
import javax.el.MethodExpression;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import javax.faces.event.AbortProcessingException;
import javax.faces.event.ActionEvent;
import javax.faces.event.ActionListener;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class ButtonHandler extends TagHandler {

  private final TagAttribute title;
  private final TagAttribute actionListener;

  public ButtonHandler(TagConfig config) {
    super(config);

    title = getRequiredAttribute("title");
    actionListener = getRequiredAttribute("actionListener");
  }  

  public void apply(FaceletContext faceletContext, UIComponent parent) throws IOException, FacesException, FaceletException, ELException {
    Button b = new Button();
    b.setTitle(title.getValue(faceletContext));
    b.setAction(actionListener.getValue());
    ((Buttons)parent).addElement(b);
  }
}
