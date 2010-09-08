package ru.novosoft.smsc.admin.logging;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class LoggerTest {

  @Test
  public void copyTest() {
    Logger l = new Logger();
    l.setLevel(Logger.Level.FATAL);
    Logger l1 = new Logger(l);

    assertEquals(l.getLevel(), l1.getLevel());
  }
}
