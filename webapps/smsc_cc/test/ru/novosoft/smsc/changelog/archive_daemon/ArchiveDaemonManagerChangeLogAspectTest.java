package ru.novosoft.smsc.changelog.archive_daemon;

import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.changelog.BulkChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonManagerChangeLogAspectTest {

  ArchiveDaemonManager manager;
  ChangeLogListenerImpl l;

  @Before
  public void before() throws AdminException {
    AdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    l = new ChangeLogListenerImpl();
    cl.addListener(l);

    manager = ctx.getArchiveDaemonManager();
  }

  @Test
  public void testLogSetter() throws Exception {
    int oldValue = manager.getIndexatorMaxFlushSpeed();
    manager.setIndexatorMaxFlushSpeed(oldValue + 1);

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.ARCHIVE_DAEMON, l.subject);
    assertEquals("Config", l.object);
    assertEquals("indexatorMaxFlushSpeed", l.propertyName);
    assertEquals(oldValue, l.oldValue);
    assertEquals(oldValue+1, l.newValue);
  }

  @Test
  public void logResetTest() throws AdminException {
    manager.reset();

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.ARCHIVE_DAEMON, l.subject);
    assertTrue(l.resetCalled);
  }

  @Test
  public void logApplyTest() throws AdminException {
    manager.apply();

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.ARCHIVE_DAEMON, l.subject);
    assertTrue(l.applyCalled);
  }

  private static class ChangeLogListenerImpl extends BulkChangeLogListener {

    int calls;
    ChangeLog.Subject subject;
    String object;
    String propertyName;
    Object oldValue;
    Object newValue;

    boolean applyCalled;
    boolean resetCalled;

    public void propertyChanged(ChangeLog.Subject subject, String object, Class objectClass, String propertyName, Object oldValue, Object newValue) {
      calls++;
      this.subject = subject;
      this.object = object;
      this.propertyName = propertyName;
      this.oldValue = oldValue;
      this.newValue = newValue;
    }

    public void applyCalled(ChangeLog.Subject subject) {
      calls++;
      this.subject = subject;
      applyCalled = true;
    }

    public void resetCalled(ChangeLog.Subject subject) {
      calls++;
      this.subject = subject;
      resetCalled = true;
    }
  }
}
