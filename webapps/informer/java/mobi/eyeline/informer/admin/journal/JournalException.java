package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Ошибка при работе с журналом
 * @author Aleksandr Khalitov
 */
public class JournalException extends AdminException{

  protected JournalException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected JournalException(String key, String... args) {
    super(key, args);
  }
}
