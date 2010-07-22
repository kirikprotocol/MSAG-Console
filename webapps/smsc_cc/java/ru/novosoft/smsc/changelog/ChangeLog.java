package ru.novosoft.smsc.changelog;

import ru.novosoft.smsc.admin.AdminContext;

import java.util.ArrayList;
import java.util.List;

/**
 * Класс, позволяющий отслеживать изменения в конкретном экземпляре AdminContext.
 * При помощи аспектов-адаптеров, ChangeLog перехватывает и интерпретирует вызова API и оповещает о них
 * зарегистрированных listener-ов.
 *
 * Методы листенеров дергаются в том же потоке, в котором производится вызов API.
 * Т.е., например, если поток А дергает метод SmscManager.setCommonSettings(),
 * то в этом же потоке произойдет вызов всех листенеров ChangeLog-а, который отновится к
 * AdminContext, содержащему данный SmscManager. 
 *
 * @author Artem Snopkov
 */
public class ChangeLog {

  static List<ChangeLog> instances = new ArrayList<ChangeLog>();

  static void addInstance(AdminContext context) {
    ChangeLog instance = getInstance(context);
    if (instance == null)
      instances.add(new ChangeLog(context));
  }

  /**
   * Возвращает экземпляр класса ChangeLog по экземпляру класса AdminContext
   * @param context экземпляр AdminContext
   * @return экземпляр класса ChangeLog
   */
  public static ChangeLog getInstance(AdminContext context) {
    for (ChangeLog instance : instances) {
      if (instance.adminContext == context)
        return instance;
    }
    return null;
  }


  private final AdminContext adminContext;
  private final List<ChangeLogListener> listeners = new ArrayList<ChangeLogListener>();


  ChangeLog(AdminContext adminContext) {
    this.adminContext = adminContext;
  }

  /**
   * Возвращает экземпляр AdminContext, к которому привязан данный ChangeLog
   * @return экземпляр AdminContext, к которому привязан данный ChangeLog
   */
  public AdminContext getAdminContext() {
    return adminContext;
  }

  /**
   * Регистрирует нового листенера
   * @param listener listener, которого надо зарегистрировать
   */
  public void addListener(ChangeLogListener listener) {
    listeners.add(listener);
  }

  /**
   * Оповещает листенеров о том, что произошло изменение свойства
   * @param source источник сообщения
   * @param object название объекта, в котором произошли изменения
   * @param objectClass класс объекта
   * @param propertyName название свойства
   * @param oldValue старое значение
   * @param newValue новое значение
   */
  public void propertyChanged(Source source, String object, Class objectClass, String propertyName, Object oldValue, Object newValue) {
    for (ChangeLogListener l : listeners)
      l.propertyChanged(source, object, objectClass, propertyName, oldValue, newValue);
  }

  /**
   * Оповещает листенеров о том, что добавился объект (например, маршрут, алиас и т.п.)
   * @param source источник сообщения
   * @param object название нового объекта
   */
  public void objectAdded(Source source, Object object) {
    for (ChangeLogListener l : listeners)
      l.objectAdded(source, object);
  }

  /**
   * Оповещает листенеров об удалении объекта (например, маршрута, алиаса и т.п.)
   * @param source источник сообщения
   * @param object инстанц объекта
   */
  public void objectRemoved(Source source, Object object) {
    for (ChangeLogListener l : listeners)
      l.objectRemoved(source, object);
  }

  /**
   * Перечисление всех возможных источников, из которых приходят ивенты
   */
  public enum Source {
    SMSC, ALIAS
  }
}
