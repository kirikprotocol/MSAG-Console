package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.util.Address;

import java.io.File;
import java.util.Collection;

/**
 * Класс для отправки комманд в ClusterController.
 *
 * @author Artem Snopkov
 */
public class ClusterController {

  public static final String SERVICE_ID = "ClusterController";


  private ServiceManager serviceManager;
  private int port;

  protected ClusterController() {
    
  }

  public ClusterController(ServiceManager serviceManager, File configFile, FileSystem fileSystem) throws AdminException {
    this.serviceManager = serviceManager;

    ServiceInfo si = getInfo();
    if (!fileSystem.exists(configFile))
      throw new ClusterControllerException("config_file_not_found", configFile.getAbsolutePath());


    //todo read port from config file
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new ClusterControllerException("cluster_controller_offline");
    return si;
  }

  public boolean isOnline() throws AdminException {
    return getInfo().getOnlineHost() != null;
  }

  /**
   * Отправляет команду на добавления алиаса
   *
   * @param address   адрес
   * @param alias     алиас
   * @param aliasHide признак скрытности алиаса
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addAlias(String address, String alias, boolean aliasHide) throws AdminException {
  }

  /**
   * Отправляет команду на удаление алиаса
   *
   * @param alias алиас
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void delAlias(String alias) throws AdminException {
  }

  /**
   * Отправляет команду на добавление закрытой группы
   * @param groupId идентификатор группы
   * @param groupName название группы
   * @param masks список масок
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addClosedGroup(long groupId, String groupName, Collection<Address> masks) throws AdminException {
  }

  /**
   * Отправляет команду на удаление закрытой группы
   * @param groupId идентификатор группы
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeClosedGroup(long groupId) throws AdminException {
  }

  /**
   * Отправляет команду на добавление масок в закрытую группу
   * @param groupId идентификатор группы
   * @param masks список масок
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addMasksToClosedGroup(long groupId, Collection<Address> masks) throws AdminException {
  }

  /**
   * Отправляет команду на удаление масок из закрытой группы
   * @param groupId идентификатор группы
   * @param masks список масок
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeMasksFromClosedGroup(long groupId, Collection<Address> masks) throws AdminException {
  }

  /**
   * Возвращает список MSC адресов, зарегистрированных в СС
   * @return список MSC адресов, зарегистрированных в СС
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public Collection<Address> getMscs() throws AdminException{
    return null;
  }

  /**
   * Регистрирует новый MSC адрес в СС
   * @param mscAddress новый MSC адрес
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void registerMsc(Address mscAddress) throws AdminException{
  }

  /**
   * Удаляет MSC адрес из реестра СС
   * @param mscAddress MSC адрес, который надо удалить из реестра
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unregisterMsc(Address mscAddress) throws AdminException{
  }

  /**
   * Отправляет команду на применение изменений в конфигурации политик передоставки
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyReschedule() throws AdminException {    
  }

  /**
   * Отправляет команду на применение изменений в конфигурации fraud
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyFraud() throws AdminException {
  }

  /**
   * Отправляет команду на применение изменений в Map Limits
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyMapLimits() throws AdminException {
    /**
   * Отправляет команду на применение изменений в конфиге SNMP
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  }

  public void applySnmp() throws AdminException {    
  }
}
