package mobi.eyeline.informer.admin.siebel;

import java.util.Map;
import java.util.Properties;

/**
 * Источник рассылок Siebel
 * @author Aleksandr Khalitov
 */
public interface SiebelDataProvider {

  /**
   * Возвращает сообщение по его идентификатору
   * @param clcId идентификатор сообщения
   * @return сообщение
   * @throws SiebelException ошибка выполнения команды
   */
  public SiebelMessage getMessage(String clcId) throws SiebelException;

  /**
   * Возвращает сообщения по идентификатору рассылки
   * @param waveId идентификатор рассылки
   * @return сообщения
   * @throws SiebelException ошибка выполнения команды
   */
  public ResultSet<SiebelMessage> getMessages(String waveId) throws SiebelException;
  
  /**
   * Возвращает состояние сообщения
   * @param clcId идентификатор рассылки
   * @return состояние рассылки
   * @throws SiebelException ошибка выполнения команды
   */
  public SiebelMessage.State getMessageState(String clcId) throws SiebelException;

  /**
   * Устанавливает состояние сообщений
   * @param deliveryStates состояния сообщений, ключ - идентификатор сообщения
   * @throws SiebelException ошибка выполнения команды
   */
  public void setMessageStates(Map<String, SiebelMessage.DeliveryState> deliveryStates) throws SiebelException;

  /**
   * Возвращает рассылку по её идентификатору
   * @param waveId идентификатор рассылки
   * @return рассылка
   * @throws SiebelException ошибка выполнения команды
   */
  public SiebelDelivery getDelivery(String waveId) throws SiebelException;

  /**
   * Возвращает рассылки, статус которых был изменён извне
   * @return рассылки
   * @throws SiebelException ошибка выполнения команды
   */
  public ResultSet<SiebelDelivery> getDeliveriesToUpdate() throws SiebelException;

  /**
   * Возвращает все рассылки
   * @return рассылки
   * @throws SiebelException ошибка выполнения команды
   */
  public ResultSet<SiebelDelivery> getDeliveries() throws SiebelException;

  /**
   * Устанавливает статус рассылки
   * @param waveId идентификатор рассылки
   * @param status новый статус
   * @throws SiebelException ошибка выполнения команды
   */
  public void setDeliveryStatus(String waveId, SiebelDelivery.Status status) throws SiebelException;

  /**
   * Возвращает статус рассылки
   * @param waveId идентифкатор рассылки
   * @return статус
   * @throws SiebelException ошибка выполнения команды
   */
  public SiebelDelivery.Status getDeliveryStatus(String waveId) throws SiebelException;

  /**
   * Есть ли у рассылки сообщения, которые находятся в не заключительном состоянии
   * @param waveId идентификатор рассылки
   * @return true - да, false - нет
   * @throws SiebelException ошибка выполнения команды
   */
  public boolean containsUnfinished(String waveId) throws SiebelException;

  /**
   * Устанавливает соединение с источником данных
   * @param props параметры соеддинения
   * @throws SiebelException ошибка выполнения команды
   */
  public void connect(Properties props) throws SiebelException;

  /**
   * Было ли выполнено завершение работы с источником данных
   * @return true - да, false - нет
   */
  public boolean isShutdowned();

  /**
   * Завершение работы с источником данных
   */
  public void shutdown();
}
