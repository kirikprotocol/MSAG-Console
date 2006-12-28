package ru.novosoft.smsc.jsp.util.tables.table;

/**
 * User: artem
 * Date: 20.12.2006
 */
final class UniqueStringGenerator {
  private static final String BASE = "n";
  private static long counter = 0;

  static String generateUniqueString() {
    counter++;
    return BASE + counter;
  }
}
