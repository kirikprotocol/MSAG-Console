package ru.novosoft.smsc.admin.msc;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;

/**
 * @author Artem Snopkov
 */
public class MscManagerTest {

  @Test(expected = IllegalArgumentException.class)
  public void addNullMscTest() throws AdminException {
    MscManager m = new MscManager(new TestClusterController());
    m.addMsc(null);
  }

  @Test(expected = IllegalArgumentException.class)
  public void removeNullMscTest() throws AdminException {
    MscManager m = new MscManager(new TestClusterController());    
    m.removeMsc(null);
  }
}
