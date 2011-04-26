package mobi.eyeline.informer.admin.pvss;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.service.ServiceInfo;
import mobi.eyeline.informer.admin.service.ServiceManager;

import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class PVSSManager {

  private final ServiceManager serviceManager;

  public final static String SERVICE_ID = "pvss";


  public PVSSManager(ServiceManager serviceManager) {
    this.serviceManager = serviceManager;
  }


  /**
   * Запуск PVSS
   *
   * @throws AdminException ошибка при запуске
   */
  public void startPVSS() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  /**
   * Остановка PVSS
   *
   * @throws AdminException ошибка при остановке
   */
  public void stopPVSS() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  /**
   * Переключение PVSS на другую ноду
   *
   * @param toHost хост
   * @throws AdminException ошибка при переключении
   */
  public void switchPVSS(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  /**
   * Возвращает хост, на котором запущен PVSS
   *
   * @return хост
   * @throws AdminException ошибка чтения статуса PVSS
   */
  public String getPVSSOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  /**
   * Возвращает список всех нод, на которых может быть запущен PVSS
   *
   * @return список нод
   * @throws AdminException ошибка чтения статуса PVSS
   */
  public List<String> getPVSSHosts() throws AdminException {
    return getInfo().getHosts();
  }



  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new PVSSException("service_manager_offline");
    return si;
  }
}
