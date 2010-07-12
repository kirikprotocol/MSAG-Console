package ru.novosoft.smsc.admin.reschedule;

import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;

import java.util.regex.Pattern;

/**
 * @author Artem Snopkov
 */
public class RescheduleTest {

  @Test
  public void invalidIntervalsTest() {
    // Корректное значение
    try {
      new Reschedule("10s,15m:3,16h:2,123d:*", 12);
    } catch (AdminException e) {
      assertFalse(true);
    }

    // Некорректные значения
    try {
      new Reschedule("10f,15m:3,25h:6,67d:*", 12);
      assertFalse(true);
    } catch (AdminException e) {}

    try {
      new Reschedule("10d:*,15m:3,25h:6,67d:*", 12);
      assertFalse(true);
    } catch (AdminException e) {}

    try {
      new Reschedule("10f,15m:3,25h:6,", 12);
      assertFalse(true);
    } catch (AdminException e) {}

    try {
      new Reschedule("10f,15m:3,25h:6,67d:*", 12);
      assertFalse(true);
    } catch (AdminException e) {}
  }
}
