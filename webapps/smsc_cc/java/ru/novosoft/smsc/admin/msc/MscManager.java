package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class MscManager implements SmscConfiguration {

  private final ClusterController cc;
  private final File aliasesFile;
  private final FileSystem fs;

  public MscManager(File aliasesFile, ClusterController cc, FileSystem fs) {
    this.cc = cc;
    this.aliasesFile = aliasesFile;
    this.fs = fs;
  }

  public Collection<Address> mscs() throws AdminException {
    BufferedReader is = null;
    try {
      if (cc.isOnline())
        cc.lockMsc(false);
      is = new BufferedReader(new InputStreamReader(fs.getInputStream(aliasesFile)));

      Collection<Address> result = new ArrayList<Address>();
      String line;
      while ((line = is.readLine()) != null)
        result.add(new Address(line));

      return result;

    } catch (IOException e) {
      throw new MscException("msc_file_read_error", e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
      if (cc.isOnline())
        cc.unlockMsc();
    }
  }

  public void addMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");

    cc.registerMsc(new Address(msc));
  }

  public void removeMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");
    cc.unregisterMsc(new Address(msc));
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState configState = cc.getMscConfigState();

    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();

    if (configState != null) {
      long ccLastUpdateTime = configState.getCcLastUpdateTime();
      for (Map.Entry<Integer, Long> e : configState.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = (e.getValue() >= ccLastUpdateTime) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }
    return result;
  }
}
