package ru.novosoft.smsc.web.components.buttons;

import javax.faces.component.UICommand;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Buttons extends UICommand {

  private List elements = new ArrayList();
  private String selectedButton;

  public String getFamily() {
    return "Eyeline";
  }

  public void addElement(Object element) {
    elements.add(element);
  }

  public List getElements() {
    return elements;
  }

  public String getSelectedButton() {
    return selectedButton;
  }

  public void setSelectedButton(String selectedButton) {
    this.selectedButton = selectedButton;
  }
}
