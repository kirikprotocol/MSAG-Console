package ru.novosoft.smsc.changelog.closed_groups;

import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class ClosedGroupChangeLogAspectTest {

  ClosedGroup cg;
  TestChangeLogListener l;


  @Before
  public void setUp() throws Exception {
    AdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    l = new TestChangeLogListener();
    cl.addListener(l);

    ClosedGroupManager m = ctx.getClosedGroupManager();
    for (ClosedGroup c : m.groups()) {
      if (c.getId() == 2)
        cg = c;
    }
  }

  @Test
  public void testLogSetDescription() throws Exception {
    String oldDesc = cg.getDescription();
    cg.setDescription("new description");

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.CLOSED_GROUP, l.changed_subject);
    assertEquals(cg.getName(), l.changed_subjectDesc);
    assertEquals("description", l.changed_propertyName);
    assertEquals(oldDesc, l.changed_oldValue);
    assertEquals("new description", l.changed_newValue);

  }

  @Test
  public void testLogAddMask() throws Exception {

    Collection<Address> oldMasks = cg.getMasks();
    cg.addMask(new Address("+7884756283"));
    Collection<Address> newMasks = cg.getMasks();

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.CLOSED_GROUP, l.changed_subject);
    assertEquals(cg.getName(), l.changed_subjectDesc);
    assertEquals("masks", l.changed_propertyName);
    assertEquals(oldMasks, l.changed_oldValue);
    assertEquals(newMasks, l.changed_newValue);
  }

  @Test
  public void testLogRemoveMask() throws Exception {

    Collection<Address> oldMasks = cg.getMasks();
    cg.removeMask(new Address("+79495445566"));
    Collection<Address> newMasks = cg.getMasks();

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.CLOSED_GROUP, l.changed_subject);
    assertEquals(cg.getName(), l.changed_subjectDesc);
    assertEquals("masks", l.changed_propertyName);
    assertEquals(oldMasks, l.changed_oldValue);
    assertEquals(newMasks, l.changed_newValue);
  }
}
