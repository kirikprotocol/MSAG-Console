package ru.novosoft.smsc.admin.route;

/**
 * Страутура, описывающая результаты трассировка маршрута
 * @author Artem Snopkov
 */
public class RouteTrace {
  private final String routeName;
  private final String[] trace;
  private final String aliasInfo;

  RouteTrace(String routeName, String[] trace, String aliasInfo) {
    this.routeName = routeName;
    this.trace = trace;
    this.aliasInfo = aliasInfo;
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

  /**
   * Возвращает информацию о найденных алиасах
   * @return информацию о найденных алиасах
   */
  public String getAliasInfo() {
    return aliasInfo;
  }
}
