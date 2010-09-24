package mobi.eyeline.informer.web.auth;

/**
 * Аутенфикатор, который используется в модуле авторизации приложения
 * @author Aleksandr Khalitov
 */
public interface Authenticator {

  /**
   * Возвращает информацию о правах пользователя или null, если пользователь не найден с таким логином и паролем
   * @param login логин
   * @param password пароль
   * @return информация о правах пользователя
   */
  public InformerPrincipal authenticate(String login, String password);
  
}
