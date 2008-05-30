package ru.sibinco.smpp.ub_sme.util;

import java.util.regex.Pattern;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 19.11.2007
 * Time: 15:53:29
 * To change this template use File | Settings | File Templates.
 */
public class Matcher {
  private Pattern regex;
  private int group;

  public Matcher(String regex, int gropIndex) {
    this.regex = Pattern.compile(regex, Pattern.DOTALL);
    this.group = gropIndex;
  }

  public Matcher(String match) {
    if (match.indexOf('#') == -1) {
      throw new IllegalArgumentException("Argument must match \"[regex]#[group index]\"");
    } else {
      group = Integer.parseInt(match.substring(match.indexOf('#') + 1));
      this.regex = Pattern.compile(match.substring(0, match.indexOf('#')), Pattern.DOTALL);
    }
  }

  public String match(String input) {
    java.util.regex.Matcher m = regex.matcher(input);
    if (m.matches()) {
      return m.group(group);
    } else {
      return null;
    }
  }

}
