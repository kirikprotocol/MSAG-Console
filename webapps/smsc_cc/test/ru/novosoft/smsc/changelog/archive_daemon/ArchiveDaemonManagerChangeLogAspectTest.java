package ru.novosoft.smsc.changelog.archive_daemon;

import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.changelog.TestChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonManagerChangeLogAspectTest {

  ArchiveDaemonManager manager;
  TestChangeLogListener l;

  @Before
  public void before() throws AdminException {
    AdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    l = new TestChangeLogListener();
    cl.addListener(l);

    manager = ctx.getArchiveDaemonManager();
  }

  @Test
  public void testLogSetter() throws Exception {
    int oldValue = manager.getIndexatorMaxFlushSpeed();
    manager.setIndexatorMaxFlushSpeed(oldValue + 1);

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.ARCHIVE_DAEMON, l.changed_subject);
    assertEquals("Config", l.changed_subjectDesc);
    assertEquals("indexatorMaxFlushSpeed", l.changed_propertyName);
    assertEquals(oldValue, l.changed_oldValue);
    assertEquals(oldValue+1, l.changed_newValue);
  }

  @Test
  public void logResetTest() throws AdminException {
    manager.reset();

    assertEquals(1, l.reset_calls);
    assertEquals(ChangeLog.Subject.ARCHIVE_DAEMON, l.reset_subject);
  }

  @Test
  public void logApplyTest() throws AdminException {
    manager.apply();

    assertEquals(1, l.apply_calls);
    assertEquals(ChangeLog.Subject.ARCHIVE_DAEMON, l.apply_subject);
  }

}
