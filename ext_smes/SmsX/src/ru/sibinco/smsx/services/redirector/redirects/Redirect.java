package ru.sibinco.smsx.services.redirector.redirects;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class Redirect {
  private final String regex;
  private final String address;

  public Redirect(String regex, String address) {
    this.regex = regex;
    this.address = address;
  }

  public String getRegex() {
    return regex;
  }

  public String getAddress() {
    return address;
  }
}
