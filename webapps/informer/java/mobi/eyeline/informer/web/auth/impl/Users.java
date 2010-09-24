package mobi.eyeline.informer.web.auth.impl;

/**
 * Интерфейс, позволяющий получить пользователя по его логину 
 * @author Aleksandr Khalitov
 */
public interface Users {

  /**
   * Возвращает пользователя по логину
   * @param login логин
   * @return пользователь или null, если пользователя нет, или произошла ошибка
   */
  public mobi.eyeline.informer.admin.users.User getUser(String login);  

}
