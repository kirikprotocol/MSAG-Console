package ru.novosoft.smsc.admin.route;

/**
 * Страутура, описывающая результаты трассировка маршрута
 * @author Artem Snopkov
 */
public class RouteTrace {
  private final String routeName;
  private final String[] trace;

  RouteTrace(String routeName, String[] trace) {
    this.routeName = routeName;
    this.trace = trace;
  }

  /**
   * Возсращает имя найденного маршрута или null, если маршрут не найден
   * @return имя найденного маршрута или null, если маршрут не найден
   */
  public String getRouteName() {
    return routeName;
  }

  /**
   * Возвращает лог сообщений СМСЦ при трейсинге. 
   * @return лог сообщений СМСЦ при трейсинге.
   */
  public String[] getTrace() {
    return trace;
  }
}
