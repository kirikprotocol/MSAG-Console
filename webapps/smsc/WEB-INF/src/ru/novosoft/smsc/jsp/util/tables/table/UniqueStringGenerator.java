package ru.novosoft.smsc.jsp.util.tables.table;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 20.12.2006
 */
final class UniqueStringGenerator {
  private static final String UNIQUE_STRING_GENERATOR = "UniqueStringGenerator";
  private final String BASE = "n";
  private long counter = 0;

  private synchronized static UniqueStringGenerator getInstance(HttpServletRequest request) {
    UniqueStringGenerator instance = (UniqueStringGenerator)request.getSession().getAttribute(UNIQUE_STRING_GENERATOR);
    if (instance != null) {
      instance = new UniqueStringGenerator();
      request.getSession().setAttribute(UNIQUE_STRING_GENERATOR, instance);
    }

    return instance;
  }

  static String generateUniqueString(HttpServletRequest request) {
    return getInstance(request).generateUniqueStringInternal();
  }

  private String generateUniqueStringInternal() {
    counter++;
    return BASE + counter;
  }
}
