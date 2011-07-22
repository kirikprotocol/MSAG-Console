package mobi.eyeline.util.jsf.components.sample.controllers;

/**
 * author: Aleksandr Khalitov
 */
public class ButtonController {

  private String button;

  public String button1() {
    button = "Button1";
    return null;
  }

  public String button2() {
    button = "Button2";
    return null;
  }

  public String button3() {
    button = "Button3";
    return null;
  }

  public String getButton() {
    return button;
  }
}
