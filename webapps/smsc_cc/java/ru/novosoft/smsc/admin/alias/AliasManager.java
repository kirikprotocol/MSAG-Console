package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.RuntimeConfiguration;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Класс для доступа к списку алиасов
 * @author Artem Snopkov
 */
public class AliasManager implements RuntimeConfiguration {

  private static final int MSG_SIZE = 48; //1+(1+1+21)+(1+1+21)+1

  private final ReadWriteLock rwlock = new ReentrantReadWriteLock();
  private final ClusterController cc;
  private final FileSystem fileSystem;
  private final File aliasesFile;

  protected AliasManager() {
    this(null, null, null);
  }

  public AliasManager(File aliasesFile, ClusterController clusterController, FileSystem fileSystem) {
    this.aliasesFile = aliasesFile;
    this.cc = clusterController;
    this.fileSystem = fileSystem;
  }


  /**
   * Добавляет/обновляет алиас.
   *
   * @param alias алиас
   * @throws AdminException если произошла ошибка
   */
  public void addAlias(Alias alias) throws AdminException {
    try {
      rwlock.writeLock().lock();
      cc.addAlias(alias.getAddress().getNormalizedAddress(), alias.getAlias().getNormalizedAddress(), alias.isHide());
    } finally {
      rwlock.writeLock().unlock();
    }
  }

  /**
   * Удаляет алиас
   *
   * @param alias алиас
   * @throws AdminException если произошла ошибка
   */
  public void deleteAlias(Alias alias) throws AdminException {
    try {
      rwlock.writeLock().lock();
      cc.delAlias(alias.getAlias().getNormalizedAddress());
    } finally {
      rwlock.writeLock().unlock();
    }
  }


  /**
   * Возвращает экземпляр AliasSet, с помощью которого можно итерироваться по алиасам.
   * @return экземпляр AliasSet, с помощью которого можно итерироваться по алиасам.
   * @throws AdminException если произошла ошибка
   */
  public AliasSet getAliases() throws AdminException {
    // todo may be lock aliases.bin file???
    try {
      rwlock.readLock().lock();
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

    public void close() {
      try {
        is.close();
      } catch (IOException ignored) {
      }
      rwlock.readLock().unlock();
    }
  }

}
