package mobi.eyeline.informer.admin.siebel;

/**
 * Множество данных
 * @author Aleksandr Khalitov
 */
public interface ResultSet<T> {

  /**
   * Если ли ещё данные в этом множестве
   * @return true - да, false - нет
   * @throws SiebelException ошибка извлечения данных
   */
  public boolean next() throws SiebelException;

  /**
   * Возвращает следующий объект данных
   * @return объект данных
   * @throws SiebelException ошибка извлечения данных
   */
  public T get() throws SiebelException;

  /**
   * Завершение работы с данными
   */
  public void close();

}
