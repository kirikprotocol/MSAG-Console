package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.service.ServiceInfo;
import mobi.eyeline.informer.admin.service.ServiceManager;

import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWServiceManager {


  private final ServiceManager serviceManager;

  public final static String SERVICE_ID = "dcpgw";


  public SmppGWServiceManager(ServiceManager serviceManager) {
    this.serviceManager = serviceManager;
  }


  /**
   * Запуск SmppGW
   *
   * @throws mobi.eyeline.informer.admin.AdminException ошибка при запуске
   */
  public void startSmppGW() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  /**
   * Остановка SmppGW
   *
   * @throws AdminException ошибка при остановке
   */
  public void stopSmppGW() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  /**
   * Переключение SmppGW на другую ноду
   *
   * @param toHost хост
   * @throws AdminException ошибка при переключении
   */
  public void switchSmppGW(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  /**
   * Возвращает хост, на котором запущен SmppGW
   *
   * @return хост
   * @throws AdminException ошибка чтения статуса SmppGW
   */
  public String getSmppGWOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  /**
   * Возвращает список всех нод, на которых может быть запущен SmppGW
   *
   * @return список нод
   * @throws AdminException ошибка чтения статуса SmppGW
   */
  public List<String> getSmppGWHosts() throws AdminException {
    return getInfo().getHosts();
  }



  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new SmppGWException("service_manager_offline");
    return si;
  }
}
