package ru.novosoft.smsc.changelog.smsc;

import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.changelog.BulkChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogListener;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class SmscManagerChangeLogAspectTest {

  SmscManager manager;
  ChangeLogListenerImpl l;

  @Before
  public void before() throws AdminException {
    AdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    l = new ChangeLogListenerImpl();
    cl.addListener(l);

    manager = ctx.getSmscManager();
  }

  @Test
  public void setCommonSettingsTest() throws AdminException {
    CommonSettings cs = manager.getCommonSettings();
    int oldAbInfoProtocolId = cs.getAbInfoProtocolId();
    cs.setAbInfoProtocolId(oldAbInfoProtocolId + 1);
    manager.setCommonSettings(cs);

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.SMSC, l.subject);
    assertEquals("Common settings", l.object);
    assertEquals("abInfoProtocolId", l.propertyName);
    assertEquals(oldAbInfoProtocolId, l.oldValue);
    assertEquals(oldAbInfoProtocolId + 1, l.newValue);
  }

  @Test
  public void setInstanceSettingsTest() throws AdminException {
    InstanceSettings cs = manager.getInstanceSettings(0);

    int oldAdminPort = cs.getAdminPort();
    cs.setAdminPort(oldAdminPort + 1);
    manager.setInstanceSettings(0, cs);

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.SMSC, l.subject);
    assertEquals("Instance 0 settings", l.object);
    assertEquals("adminPort", l.propertyName);
    assertEquals(oldAdminPort, l.oldValue);
    assertEquals(oldAdminPort + 1, l.newValue);
  }

  @Test
  public void logResetTest() throws AdminException {
    manager.reset();

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.SMSC, l.subject);
    assertTrue(l.resetCalled);
  }

  @Test
  public void logApplyTest() throws AdminException {
    manager.apply();

    assertEquals(1, l.calls);
    assertEquals(ChangeLog.Subject.SMSC, l.subject);
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
