package ru.sibinco.automation;

import java.util.*;

/**
 * User: artem
 * Date: 23.10.2006
 */

class Mask {
  private String maskBase;
  private String mask;
  private final Set numbers = new HashSet();

  public Mask(String mask) {
    this.mask = mask;

    final StringTokenizer t = new StringTokenizer(mask, ",");
    String st = t.nextToken();
    processFirstToken(st.trim());

    while (t.hasMoreTokens())
      processNextToken(t.nextToken().trim());

  }

  private void processNextToken(String token) {
    for (Iterator iterator = getNumbers(token).iterator(); iterator.hasNext();)
      addNumber((String)iterator.next());
  }

  public String getMask() {
    return mask;
  }

  private static ArrayList getNumbers(String templ) {
    ArrayList result = new ArrayList();
    if (templ.indexOf("-") < 0)
      result.add(templ);
    else {
      String left = templ.substring(0, templ.indexOf("-"));
      String right = templ.substring(templ.indexOf("-") + 1);

      if (right.length() < left.length())
        right = left.substring(0, left.length() - right.length()) + right;

      for (int i=Integer.parseInt(left); i <= Integer.parseInt(right); i++)
        result.add(String.valueOf(i));
    }
    return result;
  }

  private void addNumber(String endOfNumber) {
    if (endOfNumber.length() > maskBase.length()) {
      numbers.add(endOfNumber);
      return;
    }
    String s2 = maskBase.substring(0, maskBase.length() - endOfNumber.length() );
    numbers.add(s2.trim() + endOfNumber);
  }

  private void processFirstToken(String token) {
    final ArrayList nums = getNumbers(token);
    maskBase = (String)nums.get(0);
    for (int i=0; i< nums.size(); i++)
      addNumber((String)nums.get(i));
  }

  public Set getNumbers() {
    return numbers;
  }

  public static void main(String args[]) {
    final Set nums = new Mask("91669").getNumbers();

    for (Iterator iter = nums.iterator(); iter.hasNext();)
      System.out.println(iter.next());
  }

}
