package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Класс для доступа к списку алиасов
 *
 * @author Artem Snopkov
 */
public class AliasManagerImpl implements AliasManager {

  private static final int MSG_SIZE = 48; //1+(1+1+21)+(1+1+21)+1

  private final ReadWriteLock rwlock = new ReentrantReadWriteLock();
  private final ClusterController cc;
  private final FileSystem fileSystem;
  private final File aliasesFile;

  public AliasManagerImpl(File aliasesFile, ClusterController clusterController, FileSystem fileSystem) {
    this.aliasesFile = aliasesFile;
    this.cc = clusterController;
    this.fileSystem = fileSystem;
  }

  /**
   * Проверяет актуальность конфигурации во всех инстанцах СМСЦ
   *
   * @return true, если все инстанцы СМСЦ используют актуальную конфигурацию
   * @throws AdminException если произошла ошибка
   */
  private boolean isConsistent() throws AdminException {
    Map<Integer, SmscConfigurationStatus> statuses = getStatusForSmscs();
    return !statuses.containsValue(SmscConfigurationStatus.OUT_OF_DATE);
  }

  public void addAlias(Alias alias) throws AdminException {
    try {
      rwlock.writeLock().lock();
      if (!isConsistent())
        throw new AliasManagerException("config_is_not_consistent");
      cc.addAlias(alias.getAddress(), alias.getAlias(), alias.isHide());
    } finally {
      rwlock.writeLock().unlock();
    }
  }

  public void deleteAlias(Address alias) throws AdminException {
    try {
      rwlock.writeLock().lock();
      if (!isConsistent())
        throw new AliasManagerException("config_is_not_consistent");
      cc.delAlias(alias);
    } finally {
      rwlock.writeLock().unlock();
    }
  }
  
  public AliasSet getAliases() throws AdminException {
    try {
      rwlock.readLock().lock();
      cc.lockAliases(false);
      return new AliasSetImpl(aliasesFile);
    } catch (AdminException e) {
      rwlock.readLock().unlock();
      throw e;
    }
  }

  private class AliasSetImpl implements AliasSet {

    private final InputStream is;
    private final byte[] buffer = new byte[MSG_SIZE - 1];

    private AliasSetImpl(File aliasesFile) throws AdminException {
      this.is = new BufferedInputStream(fileSystem.getInputStream(aliasesFile));
      try {
        IOUtils.readString(is, 8); // file name
        IOUtils.readUInt32(is); // version (int)
      } catch (IOException e) {
        throw new AliasManagerException("broken_aliases_file", e);
      }
    }

    public boolean next() {
      try {
        while (IOUtils.readUInt8(is) == 0)
          IOUtils.skip(is, MSG_SIZE - 1);

        IOUtils.readFully(is, buffer);

      } catch (IOException e) {
        return false;
      }
      return true;
    }

    public Alias get() {
      InputStream is = new ByteArrayInputStream(buffer);
      try {
        byte addrTon = (byte) IOUtils.readUInt8(is);
        byte addrNpi = (byte) IOUtils.readUInt8(is);
        String addrValue = IOUtils.readString(is, 21);
        byte aliasTon = (byte) IOUtils.readUInt8(is);
        byte aliasNpi = (byte) IOUtils.readUInt8(is);
        String aliasValue = IOUtils.readString(is, 21);
        byte isHide = (byte) IOUtils.readUInt8(is);

        return new Alias(new Address(addrTon, addrNpi, addrValue), new Address(aliasTon, aliasNpi, aliasValue), isHide != 0);
      } catch (IOException e) {
        return null;
      }
    }

    public void close() throws AdminException {
      try {
        is.close();
      } catch (IOException ignored) {
      }
      rwlock.readLock().unlock();
      cc.unlockAliases();
    }
  }


  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState configState = cc.getAliasesConfigState();

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
