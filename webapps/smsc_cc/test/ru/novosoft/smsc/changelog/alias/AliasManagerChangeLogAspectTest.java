package ru.novosoft.smsc.changelog.alias;

import org.junit.Test;
import org.junit.Before;
import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.TestChangeLogListener;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public class AliasManagerChangeLogAspectTest {

  AdminContext ctx;
  TestChangeLogListener l;

  @Before
  public void before() throws AdminException {
    ctx = new TestAdminContext();

    ChangeLog changeLog = ChangeLog.getInstance(ctx);
    assertNotNull(changeLog);

    l = new TestChangeLogListener();
    changeLog.addListener(l);
  }

  @Test
  public void testLogAddAlias() throws Exception {
    Alias a = new Alias(new Address("79139494484"), new Address("679876"), false);
    ctx.getAliasManager().addAlias(a);

    assertEquals(1, l.added_calls);
    assertEquals(ChangeLog.Subject.ALIAS, l.added_subject);
    assertNotNull(l.added_object);
    assertEquals(a, l.added_object);
  }

  @Test
  public void testLogDeleteAlias() throws Exception {
    Alias a = new Alias(new Address("79139494484"), new Address("679876"), false);
    ctx.getAliasManager().deleteAlias(a);

    assertEquals(1, l.removed_calls);
    assertEquals(ChangeLog.Subject.ALIAS, l.removed_subject);
    assertNotNull(l.removed_object);
    assertEquals(a, l.removed_object);
  }
}
