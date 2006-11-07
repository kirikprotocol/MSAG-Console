package ru.sibinco.automation;

import java.util.*;

/**
 * User: artem
 * Date: 23.10.2006
 */

public final class AutomationTest {

  public static void main(String[] args) {
    singleTest();
    randomTest();
  }

  private static void singleTest() {
    final ArrayList strings = new ArrayList();
    strings.addAll(new ArrayList(new Mask("81292-91192").getNumbers()));
    Automation a = AutomationCreator.createAutomation(strings);
    System.out.println("Result: " + a.size());
    for (Iterator iter = new Mask("81292-91192").getNumbers().iterator(); iter.hasNext();) {
      String str = (String)iter.next();
      if (!a.allowWord(str))
        System.out.println("!!! " + str + " !!!");
    }
  }

  private static void randomTest() {
    final int NUMBER_OF_TESTS = 100;
    final int NUMBER_OF_STRINGS_IN_TEST = 10000;
    final int MULTIPLIER = 100000;
    final int NUMBER_OF_STRINGS_THAT_NOT_IN_LIST = 100;

    final HashSet strings = new HashSet();
    for (int i = 0; i< NUMBER_OF_TESTS; i++) {
      // Clear strings
      strings.clear();
      // Fill strings
      for (int j = 0; j < NUMBER_OF_STRINGS_IN_TEST; j++)
        strings.add(String.valueOf(Math.round(Math.random()*MULTIPLIER)));
      // Create automation
      Automation a = AutomationCreator.createAutomation(new ArrayList(strings));

      // Test automation
      for (Iterator iter = strings.iterator(); iter.hasNext();) {
        if (!a.allowWord((String)iter.next())) {
          System.out.println("Test failed");
          return;
        }
      }

      for (int k =0; k<NUMBER_OF_STRINGS_THAT_NOT_IN_LIST; k++) {
        boolean found = false;
        String num = null;
        while (!found) {
          num = String.valueOf(Math.round(Math.random()*MULTIPLIER));
          found = !strings.contains(num);
        }
        if (a.allowWord(num)) {
          System.out.println("Test failed 1");
          return;
        }
      }
    }

    System.out.println("Success");
  }

  private static void printAutomation(Automation a) {
    printState(a.startState, "");
  }



  private static void printState(State st, String str) {
    str += "(" + st.getId() + ")";
    for (Iterator iter = st.getOutLinks().keySet().iterator(); iter.hasNext();) {
      final char ch = ((Character)iter.next()).charValue();
      for (Iterator states = st.getOutLinks(ch).iterator(); states.hasNext();) {
        printState((State)states.next(), str + "--" + ch + "--");
      }
    }
    if (st.getId() == 1)
      System.out.println(str);
  }
}
