package ru.novosoft.smsc.admin.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestSmeManager extends SmeManagerImpl {

  public TestSmeManager(File configFile, File backupDir, ClusterController cc, ServiceManager sm, FileSystem fs) throws AdminException {
    super(configFile, backupDir, cc, sm, fs);
  }

  public Map<String, SmeSmscStatuses> getSmesSmscStatuses() throws AdminException {
    Collection<String> smeIds = smes().keySet();

    int i=0;
    Map<String, SmeSmscStatuses> res = new HashMap<String, SmeSmscStatuses>();
    for (String smeId: smeIds) {
      SmeSmscStatus s = new SmeSmscStatus(0, SmeConnectStatus.CONNECTED, SmeBindMode.TRX, "localhost", "lcoalhost");
      SmeSmscStatuses st = new SmeSmscStatuses(SmeConnectType.VIA_LOAD_BALANCER, new SmeSmscStatus[]{s});
      res.put(smeId, st);
    }
    return res;
  }
}
