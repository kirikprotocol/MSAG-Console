package ru.novosoft.smsc.changelog.msc;

import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public class MscManagerChangeLogAspectTest {

  MscManager m;
  TestChangeLogListener l;

  @Before
  public void setUp() throws Exception {
    AdminContext ctx = new TestAdminContext();
    ChangeLog changeLog = ChangeLog.getInstance(ctx);

    l = new TestChangeLogListener();
    changeLog.addListener(l);

    m = ctx.getMscManager();
  }

  @Test
  public void testLogAddMsc() throws Exception {
    Address a = new Address("+79139495113");
    m.addMsc(a);

    assertEquals(1, l.added_calls);
    assertEquals(ChangeLog.Subject.MSC, l.added_subject);
    assertEquals(a, l.added_object);
  }

  @Test
  public void testLogRemoveMsc() throws Exception {
    Address a = new Address("+79139495113");
    m.removeMsc(a);

    assertEquals(1, l.removed_calls);
    assertEquals(ChangeLog.Subject.MSC, l.removed_subject);
    assertEquals(a, l.removed_object);
  }
}
