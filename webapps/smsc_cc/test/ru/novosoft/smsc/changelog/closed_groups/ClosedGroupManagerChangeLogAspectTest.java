package ru.novosoft.smsc.changelog.closed_groups;

import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;

/**
 * @author Artem Snopkov
 */
public class ClosedGroupManagerChangeLogAspectTest {
  ClosedGroupManager cgm;
  TestChangeLogListener l;


  @Before
  public void setUp() throws Exception {
    AdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    l = new TestChangeLogListener();
    cl.addListener(l);

    cgm = ctx.getClosedGroupManager();
  }

  @Test
  public void testLogAddGroup() throws Exception {
    cgm.addGroup("newGroup", "newGroupDescription");

    assertEquals(1, l.added_calls);
    assertEquals(ChangeLog.Subject.CLOSED_GROUP, l.added_subject);
    assertNotNull(l.added_object);
    assertEquals("newGroup", ((ClosedGroup)l.added_object).getName());
    assertEquals("newGroupDescription", ((ClosedGroup)l.added_object).getDescription());
  }

  @Test
  public void testLogRemoveGroup() throws Exception {
    cgm.removeGroup(2);

    assertEquals(1, l.removed_calls);
    assertEquals(ChangeLog.Subject.CLOSED_GROUP, l.removed_subject);
    assertNotNull(l.removed_object);
    assertEquals("2", ((ClosedGroup)l.removed_object).getName());
    assertEquals("234", ((ClosedGroup)l.removed_object).getDescription());
  }
}
