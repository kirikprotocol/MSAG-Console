package ru.novosoft.smsc.web.config.changelog;

import org.junit.Test;

import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public class LocalChangeLogTest {

  @Test
  public void testLogSmscChanges() throws Exception {
    // todo
  }

  @Test
  public void testLogRescheduleChanges1() throws Exception {
    RescheduleSettings oldS = TestRescheduleManager.createRescheduleSettings();
    RescheduleSettings newS = TestRescheduleManager.createRescheduleSettings();

    oldS.setDefaultReschedule("1h");
    newS.setDefaultReschedule("2h");

    LocalChangeLog cl = new LocalChangeLog();
    cl.logChanges(oldS, newS, "testUser");

    List<ChangeLogRecord> records = cl.getRecords();
    assertEquals(1, records.size());

    ChangeLogRecord r = records.get(0);
    assertEquals("testUser", r.getUser());
    assertEquals(LocalChangeLog.RESCHEDULE, r.getSubjectKey());
    assertEquals("property_changed", r.getDescriptionKey());
    assertArrayEquals(new String[]{"defaultReschedule", "1h", "2h"}, r.getDescriptionArgs());
  }

  @Test
  public void testLogRescheduleChanges2() throws Exception {
    RescheduleSettings oldS = TestRescheduleManager.createRescheduleSettings();
    RescheduleSettings newS = TestRescheduleManager.createRescheduleSettings();

    oldS.setDefaultReschedule("1h");
    {
      Collection<Reschedule> r = new ArrayList<Reschedule>();
      r.add(new Reschedule("1h", 1));
      r.add(new Reschedule("2h", 2));
      r.add(new Reschedule("3h", 3));
      oldS.setReschedules(r);
    }

    newS.setDefaultReschedule("1h");
    {
      Collection<Reschedule> r = new ArrayList<Reschedule>();
      // Для 1 не меняем политику
      // Для 2 меняем политику
      // Для 3 удаляем политику
      // Для 4 добавляем политику
      r.add(new Reschedule("1h", 1));
      r.add(new Reschedule("2h", 4));
      r.add(new Reschedule("3h", 2));
      newS.setReschedules(r);
    }

    LocalChangeLog cl = new LocalChangeLog();
    cl.logChanges(oldS, newS, "testUser");

    List<ChangeLogRecord> records = cl.getRecords();
    assertEquals(3, records.size());

    for (ChangeLogRecord r : records) {
      assertEquals("testUser", r.getUser());
      assertEquals(LocalChangeLog.RESCHEDULE, r.getSubjectKey());
      switch (r.getType()) {
        case ADD:
          assertEquals("reschedule_added", r.getDescriptionKey());
          assertArrayEquals(new String[]{"4", "2h"}, r.getDescriptionArgs());
          break;
        case REMOVE:
          assertEquals("reschedule_removed", r.getDescriptionKey());
          assertArrayEquals(new String[]{"3"}, r.getDescriptionArgs());
          break;
        case CHANGE:
          assertEquals("reschedule_changed", r.getDescriptionKey());
          assertArrayEquals(new String[]{"2", "2h", "3h"}, r.getDescriptionArgs());
          break;
      }
    }
  }
}
