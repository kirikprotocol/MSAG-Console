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
    ChangeLogListenerImpl l = new ChangeLogListenerImpl();
    cl.addListener(l);

    cl.propertyChanged(ChangeLog.Subject.SMSC, null, null, null, null, null);
    assertTrue(l.propertyChanged);

    cl.objectAdded(ChangeLog.Subject.SMSC, null);
    assertTrue(l.objectAdded);

    cl.objectRemoved(ChangeLog.Subject.SMSC, null);
    assertTrue(l.objectRemoved);
  }

  private static class ChangeLogListenerImpl extends BulkChangeLogListener {

    private boolean propertyChanged;
    private boolean objectAdded;
    private boolean objectRemoved;


    public void propertyChanged(ChangeLog.Subject subject, String object, Class objectClass, String propertyName, Object oldValue, Object newValue) {
      propertyChanged = true;
    }

    public void objectAdded(ChangeLog.Subject subject, Object object) {
      objectAdded = true;
    }

    public void objectRemoved(ChangeLog.Subject subject, Object object) {
      objectRemoved = true;
    }
  }
}
