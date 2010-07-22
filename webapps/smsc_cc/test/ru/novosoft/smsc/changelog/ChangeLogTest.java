package ru.novosoft.smsc.changelog;

import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;

/**
 * @author Artem Snopkov
 */
public class ChangeLogTest {

  @Test
  public void listenerTest() throws AdminException {
    ChangeLog cl = new ChangeLog(null);
    ChangeLogListenerImpl l = new ChangeLogListenerImpl();
    cl.addListener(l);

    cl.propertyChanged(ChangeLog.Source.SMSC, null, null, null, null, null);
    assertTrue(l.propertyChanged);

    cl.objectAdded(ChangeLog.Source.SMSC, null);
    assertTrue(l.objectAdded);

    cl.objectRemoved(ChangeLog.Source.SMSC, null);
    assertTrue(l.objectRemoved);
  }

  private static class ChangeLogListenerImpl implements ChangeLogListener {

    private boolean propertyChanged;
    private boolean objectAdded;
    private boolean objectRemoved;


    public void propertyChanged(ChangeLog.Source source, String object, Class objectClass, String propertyName, Object oldValue, Object newValue) {
      propertyChanged = true;
    }

    public void objectAdded(ChangeLog.Source source, Object object) {
      objectAdded = true;
    }

    public void objectRemoved(ChangeLog.Source source, Object object) {
      objectRemoved = true;
    }
  }
}
