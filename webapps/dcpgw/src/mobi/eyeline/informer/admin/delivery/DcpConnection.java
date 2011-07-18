package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.DcpClient;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;

import java.util.List;

import static mobi.eyeline.informer.admin.delivery.DcpConverter.convert;

/**
 * Коннект к DCP, упрвление рассылками
 *
 * @author Aleksandr Khalitov
 */
public class DcpConnection {

  private DcpClient client;

  public DcpConnection(String host, int port, final String login, String password) throws AdminException {
    this.client = new DcpClient(host, port, login, password);
  }

  protected DcpConnection() {
  }

  /**
   * Закрывает соединение
   */
  public void close() {
    client.close();
  }


  /**
   * Добавляет сообщения в рассылку
   *
   * @param deliveryId идентификатор рассылки
   * @param messages   сообщения
   * @return идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
    AddDeliveryMessages req = new AddDeliveryMessages();
    req.setDeliveryId(deliveryId);
    req.setMessages(convert(messages));
    AddDeliveryMessagesResp resp = client.send(req);
    return resp.getMessageIds();
  }

  /**
   * Меняет состояние рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @param state      новое состояние
   * @throws AdminException ошибка выполнения команды
   */
  public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {
    ChangeDeliveryState req = new ChangeDeliveryState();
    req.setDeliveryId(deliveryId);
    req.setState(convert(state));
    client.send(req);
  }

  /**
   * Возвращает статистику по рассылке
   *
   * @param deliveryId идентификатор рассылки
   * @return статистика по рассылке
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
    GetDeliveryState req = new GetDeliveryState();
    req.setDeliveryId(deliveryId);
    GetDeliveryStateResp resp;
    resp = client.send(req);
    return convert(resp.getStats(), resp.getState());
  }

}
