package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * Источник рассылок Siebel
 *
 * @author Aleksandr Khalitov
 */
interface SiebelDataProvider {

  /**
   * Возвращает сообщения по идентификатору рассылки
   *
   * @param waveId идентификатор рассылки
   * @return сообщения
   * @throws AdminException ошибка выполнения команды
   */
  public ResultSet<SiebelMessage> getMessages(String waveId) throws AdminException;

  /**
   * Устанавливает состояние сообщений
   *
   * @param deliveryStates состояния сообщений, ключ - идентификатор сообщения
   * @throws AdminException ошибка выполнения команды
   */
  public void setMessageStates(Map<String, DeliveryMessageState> deliveryStates) throws AdminException;

  /**
   * Возвращает рассылку по её идентификатору
   *
   * @param waveId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public SiebelDelivery getDelivery(String waveId) throws AdminException;

  /**
   * Возвращает рассылки, статус которых был изменён извне
   *
   * @param from дата, начиная с которой следует извлечь рассылки
   * @return рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public ResultSet<SiebelDelivery> getDeliveriesToUpdate(Date from) throws AdminException;

  /**
   * Возвращает все рассылки
   *
   * @return рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public ResultSet<SiebelDelivery> getDeliveries() throws AdminException;

  /**
   * Устанавливает статус рассылки
   *
   * @param waveId идентификатор рассылки
   * @param status новый статус
   * @throws AdminException ошибка выполнения команды
   */
  public void setDeliveryStatus(String waveId, SiebelDelivery.Status status) throws AdminException;

  /**
   * Устанавливает статусы у рассылок
   * 
   * @param statuses статус (waveId => status)
   * @throws AdminException ошибка выполнения команды
   */
  public void setDeliveryStatuses(Map<String, SiebelDelivery.Status> statuses) throws AdminException;

  /**
   * Возвращает статус рассылки
   *
   * @param waveId идентифкатор рассылки
   * @return статус
   * @throws AdminException ошибка выполнения команды
   */
  public SiebelDelivery.Status getDeliveryStatus(String waveId) throws AdminException;

  /**
   * Есть ли у рассылки сообщения, которые находятся в не заключительном состоянии
   *
   * @param waveId идентификатор рассылки
   * @return true - да, false - нет
   * @throws AdminException ошибка выполнения команды
   */
  public boolean containsUnfinished(String waveId) throws AdminException;

  /**
   * Устанавливает соединение с источником данных
   *
   * @param props параметры соеддинения
   * @throws AdminException ошибка выполнения команды
   */
  public void connect(Properties props) throws AdminException;


  /**
   * Проверяет соединение с источником данных
   *
   * @param props параметры соеддинения
   * @throws AdminException ошибка выполнения команды
   */
  public void check(Properties props) throws AdminException;

  /**
   * Было ли выполнено завершение работы с источником данных
   *
   * @return true - да, false - нет
   */
  public boolean isShutdowned();

  /**
   * Завершение работы с источником данных
   */
  public void shutdown();
}
