package mobi.eyeline.informer.admin.protogen;

import java.io.IOException;

/**
 * Сервер оффлайн
 * @author Artem Snopkov
 */
public class ServerOfflineException extends IOException {

   ServerOfflineException() {
  }

   ServerOfflineException(String message) {
    super(message);
  }

   ServerOfflineException(String message, Throwable cause) {
    super(message, cause);
  }

   ServerOfflineException(Throwable cause) {
    super(cause);
  }
}
