package ru.novosoft.smsc.web.journal;

import static org.junit.Assert.*;

import org.junit.Test;
import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class RescheduleSettingsDiffHelperTest  {
  
  @Test
  public void testLogChanges1() throws Exception {
    RescheduleSettings oldS = TestRescheduleManager.createRescheduleSettings();
    RescheduleSettings newS = TestRescheduleManager.createRescheduleSettings();

    oldS.setDefaultReschedule("1h");
    newS.setDefaultReschedule("2h");

    Journal j = new Journal();
    RescheduleSettingsDiffHelper h = new RescheduleSettingsDiffHelper(JournalRecord.Subject.RESCHEDULE);
    h.logChanges(j, oldS, newS, "testUser");

    List<JournalRecord> records = j.getRecords();
    assertEquals(1, records.size());

    JournalRecord r = records.get(0);
    assertEquals("testUser", r.getUser());
    assertEquals(JournalRecord.Subject.RESCHEDULE, r.getSubjectKey());
    assertEquals("property_changed", r.getDescriptionKey());
    assertArrayEquals(new String[]{"defaultReschedule", "1h", "2h"}, r.getDescriptionArgs());
  }
  
  @Test
  public void testLogChanges2() throws Exception {
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

    Journal journal = new Journal();
    RescheduleSettingsDiffHelper h = new RescheduleSettingsDiffHelper(JournalRecord.Subject.RESCHEDULE);
    h.logChanges(journal, oldS, newS, "testUser");

    List<JournalRecord> records = journal.getRecords();
    assertEquals(3, records.size());

    for (JournalRecord r : records) {
      assertEquals("testUser", r.getUser());
      assertEquals(JournalRecord.Subject.RESCHEDULE, r.getSubjectKey());
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
