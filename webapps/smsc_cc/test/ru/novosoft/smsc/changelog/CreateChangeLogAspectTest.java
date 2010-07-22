package ru.novosoft.smsc.changelog;

import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class CreateChangeLogAspectTest {

  @Test
  public void addChangeLogTest() throws AdminException {
    AdminContext ctx = new TestAdminContext();
    assertNotNull(ChangeLog.getInstance(ctx));
  }
}
