package ru.novosoft.smsc.changelog.smsc;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.changelog.BulkChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author Artem Snopkov
 */
public class SmscManagerChangeLogAspectTest {

  @Test
  public void setCommonSettingsTest() throws AdminException {
    TestAdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    ChangeLogListenerImpl l = new ChangeLogListenerImpl();
    cl.addListener(l);

    SmscManager manager = ctx.getSmscManager();
    CommonSettings cs = manager.getCommonSettings();
    int oldAbInfoProtocolId = cs.getAbInfoProtocolId();
    cs.setAbInfoProtocolId(oldAbInfoProtocolId + 1);
    manager.setCommonSettings(cs);

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Source.SMSC, l.source);
    assertEquals("Common settings", l.object);
    assertEquals("abInfoProtocolId", l.propertyName);
    assertEquals(oldAbInfoProtocolId, l.oldValue);
    assertEquals(oldAbInfoProtocolId+1, l.newValue);
  }

  @Test
  public void setInstanceSettingsTest() throws AdminException {
    TestAdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    ChangeLogListenerImpl l = new ChangeLogListenerImpl();
    cl.addListener(l);

    SmscManager manager = ctx.getSmscManager();
    InstanceSettings cs = manager.getInstanceSettings(0);

    int oldAdminPort = cs.getAdminPort();
    cs.setAdminPort(oldAdminPort + 1);
    manager.setInstanceSettings(0, cs);

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Source.SMSC, l.source);
    assertEquals("Instance 0 settings", l.object);
    assertEquals("adminPort", l.propertyName);
    assertEquals(oldAdminPort, l.oldValue);
    assertEquals(oldAdminPort+1, l.newValue);
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
