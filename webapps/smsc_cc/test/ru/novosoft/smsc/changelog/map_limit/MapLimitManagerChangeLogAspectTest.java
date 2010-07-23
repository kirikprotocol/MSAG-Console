package ru.novosoft.smsc.changelog.map_limit;

import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class MapLimitManagerChangeLogAspectTest {
  MapLimitManager m;
  TestChangeLogListener l;

  @Before
  public void setUp() throws Exception {
    AdminContext ctx = new TestAdminContext();
    ChangeLog changeLog = ChangeLog.getInstance(ctx);

    l = new TestChangeLogListener();
    changeLog.addListener(l);

    m = ctx.getMapLimitManager();
  }

  @Test
  public void testLogSetter() throws Exception {
    int oldDlgLimitIn = m.getDlgLimitIn();
    m.setDlgLimitIn(oldDlgLimitIn + 1);


    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.MAP_LIMIT, l.changed_subject);
    assertEquals("dlgLimitIn", l.changed_propertyName);
    assertEquals(oldDlgLimitIn, l.changed_oldValue);
    assertEquals(oldDlgLimitIn + 1, l.changed_newValue);
  }

  @Test
  public void testLogApply() throws Exception {
    m.apply();

    assertEquals(1, l.apply_calls);
    assertEquals(ChangeLog.Subject.MAP_LIMIT, l.apply_subject);
  }

  @Test
  public void testLogReset() throws Exception {
    m.reset();

    assertEquals(1, l.reset_calls);
    assertEquals(ChangeLog.Subject.MAP_LIMIT, l.reset_subject);
  }
}
