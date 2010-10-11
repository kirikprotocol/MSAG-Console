package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка при работе с СМСЦ
 * @author Aleksandr Khalitov
 */
public class SmscException extends AdminException{

  public SmscException(String key, String... args) {
    super(key, args);
  }

}
