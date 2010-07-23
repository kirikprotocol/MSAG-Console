package ru.novosoft.smsc.changelog.smsc;

import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.changelog.TestChangeLogListener;
import ru.novosoft.smsc.changelog.ChangeLog;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class SmscManagerChangeLogAspectTest {

  SmscManager manager;
  TestChangeLogListener l;

  @Before
  public void before() throws AdminException {
    AdminContext ctx = new TestAdminContext();
    ChangeLog cl = ChangeLog.getInstance(ctx);
    assertNotNull(cl);

    l = new TestChangeLogListener();
    cl.addListener(l);

    manager = ctx.getSmscManager();
  }

  @Test
  public void setCommonSettingsTest() throws AdminException {
    CommonSettings cs = manager.getCommonSettings();
    int oldAbInfoProtocolId = cs.getAbInfoProtocolId();
    cs.setAbInfoProtocolId(oldAbInfoProtocolId + 1);
    manager.setCommonSettings(cs);

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.SMSC, l.changed_subject);
    assertEquals("Common settings", l.changed_subjectDesc);
    assertEquals("abInfoProtocolId", l.changed_propertyName);
    assertEquals(oldAbInfoProtocolId, l.changed_oldValue);
    assertEquals(oldAbInfoProtocolId + 1, l.changed_newValue);
  }

  @Test
  public void setInstanceSettingsTest() throws AdminException {
    InstanceSettings cs = manager.getInstanceSettings(0);

    int oldAdminPort = cs.getAdminPort();
    cs.setAdminPort(oldAdminPort + 1);
    manager.setInstanceSettings(0, cs);

    assertEquals(1, l.changed_calls);
    assertEquals(ChangeLog.Subject.SMSC, l.changed_subject);
    assertEquals("Instance 0 settings", l.changed_subjectDesc);
    assertEquals("adminPort", l.changed_propertyName);
    assertEquals(oldAdminPort, l.changed_oldValue);
    assertEquals(oldAdminPort + 1, l.changed_newValue);
  }

  @Test
  public void logResetTest() throws AdminException {
    manager.reset();

    assertEquals(1, l.reset_calls);
    assertEquals(ChangeLog.Subject.SMSC, l.reset_subject);
  }

  @Test
  public void logApplyTest() throws AdminException {
    manager.apply();

    assertEquals(1, l.apply_calls);
    assertEquals(ChangeLog.Subject.SMSC, l.apply_subject);
  }  
}
