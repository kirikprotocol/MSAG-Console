package ru.novosoft.smsc.changelog.archive_daemon;

import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.changelog.BulkChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonManagerChangeLogAspectTest {

  @Test
  public void testLogSetter() throws Exception {
    TestAdminContext ctx = new TestAdminContext();

    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    ChangeLogListenerImpl l = new ChangeLogListenerImpl();
    cl.addListener(l);

    ArchiveDaemonManager m = ctx.getArchiveDaemonManager();
    int oldValue = m.getIndexatorMaxFlushSpeed();
    m.setIndexatorMaxFlushSpeed(oldValue + 1);

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Source.ARCHIVE_DAEMON, l.source);
    assertEquals("Config", l.object);
    assertEquals("indexatorMaxFlushSpeed", l.propertyName);
    assertEquals(oldValue, l.oldValue);
    assertEquals(oldValue+1, l.newValue);
  }

  private static class ChangeLogListenerImpl extends BulkChangeLogListener {

    int calls;
    ChangeLog.Source source;
    String object;
    String propertyName;
    Object oldValue;
    Object newValue;

    public void propertyChanged(ChangeLog.Source source, String object, Class objectClass, String propertyName, Object oldValue, Object newValue) {
      calls++;
      this.source = source;
      this.object = object;
      this.propertyName = propertyName;
      this.oldValue = oldValue;
      this.newValue = newValue;
    }
  }
}
