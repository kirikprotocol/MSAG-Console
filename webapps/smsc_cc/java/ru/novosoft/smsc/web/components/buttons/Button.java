package ru.novosoft.smsc.web.components.buttons;

import javax.el.MethodExpression;
import javax.faces.component.UICommand;

/**
 * @author Artem Snopkov
 */
public class Button {

  private String title;
  private String action;

  public String getTitle() {
    return title;
  }

  public void setTitle(String title) {
    this.title = title;
  }

  public String getAction() {
    return action;
  }

  public void setAction(String action) {
    this.action = action;
  }
}
