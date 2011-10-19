package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public class MscManagerImpl implements MscManager {

  private final ClusterController cc;
  private final File mscsFile;
  private final FileSystem fs;
  private Set<Address> mscs;
  private final Lock mscsLock = new ReentrantLock();

  public MscManagerImpl(File mscsFile, ClusterController cc, FileSystem fs) {
    this.cc = cc;
    this.mscsFile = mscsFile;
    this.fs = fs;
  }

  public Collection<Address> mscs() throws AdminException {
    try {
      mscsLock.lock();
      return new ArrayList<Address>(getMscs());
    } finally {
      mscsLock.unlock();
    }
  }

  private Collection<Address> getMscs() throws AdminException {
    if (mscs == null)
      mscs = new HashSet<Address>(load());

    return mscs;
  }

  private Collection<Address> load() throws AdminException {
    BufferedReader is = null;
    try {
      if (cc.isOnline())
        cc.lockMsc(false);
      is = new BufferedReader(new InputStreamReader(fs.getInputStream(mscsFile)));

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

  private void save() throws AdminException {
    Collection<Address> mscsList = getMscs();
    BufferedWriter os = null;
    try {
      if (cc.isOnline())
        cc.lockMsc(true);

      os = new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(mscsFile)));

      for (Address msc : mscsList) {
        os.write(msc.getSimpleAddress());
        os.write(Character.LINE_SEPARATOR);
      }

    } catch (IOException e) {
      throw new MscException("msc_file_write_error", e);
    } finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException ignored) {
        }
      if (cc.isOnline())
        cc.unlockMsc();
    }
  }

  public void addMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");

    try {
      mscsLock.lock();

      getMscs();

      mscs.add(msc);

      save();

      if (cc.isOnline())
        cc.registerMsc(new Address(msc));

    } finally {
      mscsLock.unlock();
    }
  }

  public void removeMsc(Address msc) throws AdminException {
    if (msc == null)
      throw new IllegalArgumentException("mscAddress");

    try {
      mscsLock.lock();

      getMscs();

      mscs.remove(msc);

      save();

      if (cc.isOnline())
        cc.unregisterMsc(new Address(msc));

    } finally {
      mscsLock.unlock();
    }
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState configState = cc.getMscConfigState();

    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();

    if (configState != null) {
      long ccLastUpdateTime = configState.getCcLastUpdateTime();
      for (Map.Entry<Integer, Long> e : configState.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s;
        long lastSmscUpdateTime = e.getValue();
        if (lastSmscUpdateTime == 0)
          s = SmscConfigurationStatus.NOT_SUPPORTED;
        else
          s = (e.getValue() >= ccLastUpdateTime) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }
    return result;
  }
}
