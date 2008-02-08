package ru.sibinco.smsx.engine.smpphandler.redirects;

import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class Redirect {
  private final Pattern format;
  private final Pattern prefix;
  private final String newPrefix;
  private final String address;

  public Redirect(String format, String prefix, String newPrefix, String address) {
    this.format = Pattern.compile(format, Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
    this.prefix = Pattern.compile(prefix, Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
    this.newPrefix = newPrefix;
    this.address = address;
  }

  public String getAddress() {
    return address;
  }

  public Pattern getPrefix() {
    return prefix;
  }

  public String getNewPrefix() {
    return newPrefix;
  }

  public Pattern getFormat() {
    return format;
  }
}
