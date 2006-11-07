package ru.sibinco.otasme.engine.template;

/**
 * User: artem
 * Date: 25.10.2006
 */

public final class MacroRegion {
  private final String name;
  private final String number;
  private Automation automation;

  public MacroRegion(String name, String number) {
    this.name = name;
    this.number = number;
  }

  public void setAutomation(Automation automation) {
    this.automation = automation;
  }

  public String getName() {
    return name;
  }

  public String getNumber() {
    return number;
  }

  public boolean containsNumber(String number) {
    return automation.allowWord(number);
  }
}
