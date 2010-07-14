package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;

/**
 * Класс для отправки комманд в ClusterController.
 *
 * @author Artem Snopkov
 */
public class ClusterController {

  private static final String SERVICE_ID = "ClusterController";


  private ServiceManager serviceManager;
  private int port;

  protected ClusterController() {
    
  }

  public ClusterController(ServiceManager serviceManager, FileSystem fileSystem) throws AdminException {
    this.serviceManager = serviceManager;

    ServiceInfo si = getInfo();
    File configFile = new File(si.getBaseDir(), "conf" + File.separator + "config.xml");
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

  }
}
