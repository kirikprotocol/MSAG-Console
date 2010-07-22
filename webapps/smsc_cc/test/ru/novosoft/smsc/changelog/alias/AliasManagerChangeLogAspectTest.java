package ru.novosoft.smsc.changelog.alias;

import org.junit.Test;
import org.junit.Before;
import static org.junit.Assert.*;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.changelog.BulkChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.util.Address;

/**
 * @author Artem Snopkov
 */
public class AliasManagerChangeLogAspectTest {

  AdminContext ctx;
  ChangeLogListenerImpl l;

  @Before
  public void before() throws AdminException {
    ctx = new TestAdminContext();

    ChangeLog changeLog = ChangeLog.getInstance(ctx);
    assertNotNull(changeLog);

    l = new ChangeLogListenerImpl();
    changeLog.addListener(l);
  }

  @Test
  public void testLogAddAlias() throws Exception {
    Alias a = new Alias(new Address("79139494484"), new Address("679876"), false);
    ctx.getAliasManager().addAlias(a);

    assertEquals(ChangeLog.Source.ALIAS, l.source);
    assertNotNull(l.added);
    assertEquals(a, l.added);
  }

  @Test
  public void testLogDeleteAlias() throws Exception {
    Alias a = new Alias(new Address("79139494484"), new Address("679876"), false);
    ctx.getAliasManager().deleteAlias(a);

    assertEquals(ChangeLog.Source.ALIAS, l.source);
    assertNotNull(l.removed);
    assertEquals(a, l.removed);
  }

  private static class ChangeLogListenerImpl extends BulkChangeLogListener {
    ChangeLog.Source source;
    Object added;
    Object removed;

    public void objectAdded(ChangeLog.Source source, Object object) {
      this.source = source;
      this.added = object;
    }

    public void objectRemoved(ChangeLog.Source source, Object object) {
      this.source = source;
      this.removed = object;
    }
  }
}
