package mobi.eyeline.informer.admin.delivery.stat;

/**
 * Обработчик записей статистики
 *
 * @author Artem Snopkov
 */
public interface DeliveryStatVisitor {

  /**
   * Метод описывает действия над конкретной записью статистики.
   *
   * @param rec текущая запись
   * @return true, если необходимо перейти к следующей записи или false, если процесс обработки надо остановить.
   */
  boolean visit(DeliveryStatRecord rec, int total, int current);
}
