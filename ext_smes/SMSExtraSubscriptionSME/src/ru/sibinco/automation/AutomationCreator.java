package ru.sibinco.automation;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * User: artem
 * Date: 23.10.2006
 */

final class AutomationCreator {

  /**
   * Функция строит автомат по последовательности слов. Алгоритм в упрощенном виде можно представить в виде 3-х этапов
   * 1. Построение большого автомата
   *    На этом этапе строится большой автомат, способный распознать все слова из списка. Например, для списка из 3-х
   *    слов (123, 124, 133) будет построен автомат:
   *    (start)--1--(1)--2--(2)--3--(end)
   *    (start)--1--(3)--2--(4)--4--(end)
   *    (start)--1--(5)--3--(6)--3--(end)
   *    , где "(n)" - это вершина автомата с номером n, а "--ch--" - это переход с символом ch.
   *
   * На следующих 2-х этапах происходит минимизация полученного автомата
   *
   * 2. Обратный ход
   *    Для каждой вершины определяется её уровень - т.е. максимальное количество переходов необходимое для достижения
   *    конечной вершины из данной. В примере на одном уровне находятся вершины (2,4,6) - уровень 1, (1,3,5) - уровень 2.
   *    Далее просматриваются вершины с одного уровня (уровни начинаются с 1-ого) и среди них ищутся вершины с одинаковыми
   *    множествами исходящих переходов (два исходящих перехода равны, если они указывают на одну и туже вершину и имеют
   *    один и тот же символ. Такие вершины объединяются. В нашем примере объединятся вершины с номерами 2 и 6. В итоге
   *    получится автомат:
   *    (start)--1--(1)--2--(2)--3--(end)
   *    (start)--1--(3)--2--(4)--4--(end)
   *    (start)--1--(5)--3--(2)--3--(end)
   *
   * 3. Прямой ход
   *    Теперь пробегаемся по уровням с последнего до первого. На каждом уровне ищем вершины с совпадающими множествами
   *    входящих переходов. И объединяем текие вершины. На 2-м уровне все вершины имеют одинаковые множества входящих
   *    переходов. Поэтому их объединяем:
   *    (start)--1--(1)--2--(2)--3--(end)
   *    (start)--1--(1)--2--(4)--4--(end)
   *    (start)--1--(1)--3--(2)--3--(end)
   *    Далее, на первом уровне не появились вершины, которые можно объединить.
   *    В итоге получили автомат с 5-ю состояниями.
   *
   *  Таков алгоритм в общем виде. В программе этапы 1 и 2 объединены между собой для ускорения и уменьшения требуемой
   *  памяти.
   *  Строки во входной последовательности не обязательно одинаковой длины.
   *
   * @param strings - входная последовательность строк
   * @return Конечный автомат, распознающий все строки из входной последовательности
   */
  public static Automation createAutomation(ArrayList strings) {

    // Вычисляем максимальную длину слова из списка
    int maxLength = -1;
    for (int i=0; i< strings.size(); i++)
      maxLength = (maxLength < ((String)strings.get(i)).length()) ? ((String)strings.get(i)).length() : maxLength;

    // Создаем автомат
    final Automation automation = new Automation();

    Automation.StatesLevel workingLevel, previousLevel = null;

    // Заполняем первый уровень конечными состояниями
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

          final char ch = str.charAt(str.length() - stringChar);  // Берем следующий символ

          final State state = (str.length() > stringChar) ? new State() : automation.startState; // Если этот символ является первым в слове,
                                                                                                 // то создаем ссылку из начального состояния,
                                                                                                 // иначе создаем новое состояние
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
   * Осуществляет обратный ход на одном уровне
   * @param a автомат
   * @param level уровень
   */
  private static void removeIterantStates(Automation a, int level) {
    if (level < 0)
      return;
    a.getLevel(level).removeIterantStates();
  }

  /**
   * Осуществляет прямой ход, начиная с заданного уровня
   * @param a автомат
   * @param level начальный уровень
   */
  private static void refactorStates(Automation a, int level) {
    if (level < 0)
      return;

    a.getLevel(level).processStates();
    refactorStates(a, level-1);
  }

}

