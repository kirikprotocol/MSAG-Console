package ru.novosoft.smsc.changelog;

import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ChangeLogTest {

  @Test
  public void listenerTest() throws AdminException {
    ChangeLog cl = new ChangeLog(null);
    TestChangeLogListener l = new TestChangeLogListener();
    cl.addListener(l);

    cl.propertyChanged(ChangeLog.Subject.SMSC, null, null, null, null, null);
    assertTrue(l.changed_calls == 1);

    cl.objectAdded(ChangeLog.Subject.SMSC, null);
    assertTrue(l.added_calls == 1);

    cl.objectRemoved(ChangeLog.Subject.SMSC, null);
    assertTrue(l.removed_calls == 1);

    cl.resetCalled(null);
    assertTrue(l.removed_calls == 1);

    cl.applyCalled(null);
    assertTrue(l.apply_calls == 1);
  }
}
