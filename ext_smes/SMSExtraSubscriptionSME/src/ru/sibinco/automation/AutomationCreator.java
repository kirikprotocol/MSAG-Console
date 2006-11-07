package ru.sibinco.automation;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * User: artem
 * Date: 23.10.2006
 */

final class AutomationCreator {

  /**
   * ������� ������ ������� �� ������������������ ����. �������� � ���������� ���� ����� ����������� � ���� 3-� ������
   * 1. ���������� �������� ��������
   *    �� ���� ����� �������� ������� �������, ��������� ���������� ��� ����� �� ������. ��������, ��� ������ �� 3-�
   *    ���� (123, 124, 133) ����� �������� �������:
   *    (start)--1--(1)--2--(2)--3--(end)
   *    (start)--1--(3)--2--(4)--4--(end)
   *    (start)--1--(5)--3--(6)--3--(end)
   *    , ��� "(n)" - ��� ������� �������� � ������� n, � "--ch--" - ��� ������� � �������� ch.
   *
   * �� ��������� 2-� ������ ���������� ����������� ����������� ��������
   *
   * 2. �������� ���
   *    ��� ������ ������� ������������ � ������� - �.�. ������������ ���������� ��������� ����������� ��� ����������
   *    �������� ������� �� ������. � ������� �� ����� ������ ��������� ������� (2,4,6) - ������� 1, (1,3,5) - ������� 2.
   *    ����� ��������������� ������� � ������ ������ (������ ���������� � 1-���) � ����� ��� ������ ������� � �����������
   *    ����������� ��������� ��������� (��� ��������� �������� �����, ���� ��� ��������� �� ���� � ���� ������� � �����
   *    ���� � ��� �� ������. ����� ������� ������������. � ����� ������� ����������� ������� � �������� 2 � 6. � �����
   *    ��������� �������:
   *    (start)--1--(1)--2--(2)--3--(end)
   *    (start)--1--(3)--2--(4)--4--(end)
   *    (start)--1--(5)--3--(2)--3--(end)
   *
   * 3. ������ ���
   *    ������ ����������� �� ������� � ���������� �� �������. �� ������ ������ ���� ������� � ������������ �����������
   *    �������� ���������. � ���������� ����� �������. �� 2-� ������ ��� ������� ����� ���������� ��������� ��������
   *    ���������. ������� �� ����������:
   *    (start)--1--(1)--2--(2)--3--(end)
   *    (start)--1--(1)--2--(4)--4--(end)
   *    (start)--1--(1)--3--(2)--3--(end)
   *    �����, �� ������ ������ �� ��������� �������, ������� ����� ����������.
   *    � ����� �������� ������� � 5-� �����������.
   *
   *  ����� �������� � ����� ����. � ��������� ����� 1 � 2 ���������� ����� ����� ��� ��������� � ���������� ���������
   *  ������.
   *  ������ �� ������� ������������������ �� ����������� ���������� �����.
   *
   * @param strings - ������� ������������������ �����
   * @return �������� �������, ������������ ��� ������ �� ������� ������������������
   */
  public static Automation createAutomation(ArrayList strings) {

    // ��������� ������������ ����� ����� �� ������
    int maxLength = -1;
    for (int i=0; i< strings.size(); i++)
      maxLength = (maxLength < ((String)strings.get(i)).length()) ? ((String)strings.get(i)).length() : maxLength;

    // ������� �������
    final Automation automation = new Automation();

    Automation.StatesLevel workingLevel, previousLevel = null;

    // ��������� ������ ������� ��������� �����������
    previousLevel = automation.newLevel();
    for (int i=0; i < strings.size(); i++)
      previousLevel.insertState(automation.finalState);

    int level = 0;
    int stringChar = 0;

    System.out.println("");
    System.out.println("Forward stroke...");
    while(stringChar + 1 <= maxLength) {

      System.out.print(level);
      stringChar = level + 1;

      workingLevel = automation.newLevel();

      for(int i =0; i< strings.size(); i++) {

        final String str = (String)strings.get(i);
        if (str.length() >= stringChar) {

          final char ch = str.charAt(str.length() - stringChar);  // ����� ��������� ������

          final State state = (str.length() > stringChar) ? new State() : automation.startState; // ���� ���� ������ �������� ������ � �����,
                                                                                                 // �� ������� ������ �� ���������� ���������,
                                                                                                 // ����� ������� ����� ���������
          workingLevel.insertState(state, ch, previousLevel.getState(i));

        } else {
          if (previousLevel != null)
            workingLevel.insertState(previousLevel.getState(i));
        }
      }

      removeIterantStates(automation, level-1);
      previousLevel = workingLevel;
      level++;
    }
    System.out.println("");
    System.out.println("Backward stroke...");
    refactorStates(automation, level-1);

    return automation;
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

  /**
   * ������������ �������� ��� �� ����� ������
   * @param a �������
   * @param level �������
   */
  private static void removeIterantStates(Automation a, int level) {
    if (level < 0)
      return;
    a.getLevel(level).removeIterantStates();
  }

  /**
   * ������������ ������ ���, ������� � ��������� ������
   * @param a �������
   * @param level ��������� �������
   */
  private static void refactorStates(Automation a, int level) {
    if (level < 0)
      return;

    a.getLevel(level).processStates();
    refactorStates(a, level-1);
  }

}

