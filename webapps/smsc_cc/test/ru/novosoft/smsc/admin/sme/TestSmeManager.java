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
      SmeSmscStatuses st;
      if (i % 3 == 0) {
        SmeSmscStatus s = new SmeSmscStatus(0, SmeConnectStatus.CONNECTED, SmeBindMode.TRX, "localhost", "lcoalhost");
        st = new SmeSmscStatuses(SmeConnectType.VIA_LOAD_BALANCER, new SmeSmscStatus[]{s});
        res.put(smeId, st);
      } else if (i % 3 == 1) {
        SmeSmscStatus s = new SmeSmscStatus(0, SmeConnectStatus.CONNECTED, SmeBindMode.TRX, "localhost", "lcoalhost");
        SmeSmscStatus s1 = new SmeSmscStatus(1, SmeConnectStatus.CONNECTED, SmeBindMode.TRX, "localhost", "lcoalhost");
        st = new SmeSmscStatuses(SmeConnectType.DIRECT_CONNECT, new SmeSmscStatus[]{s,s1});
        res.put(smeId, st);
      } else {
        SmeSmscStatus s = new SmeSmscStatus(0, SmeConnectStatus.INTERNAL, SmeBindMode.TRX, "localhost", "lcoalhost");
        st = new SmeSmscStatuses(SmeConnectType.DIRECT_CONNECT, new SmeSmscStatus[]{s});
      }
      res.put(smeId, st);
      i++;
    }
    return res;
  }
}
