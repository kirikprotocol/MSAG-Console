package mobi.eyeline.informer.admin.protogen;

import java.io.IOException;

/**
 * Превышен интервал ожидания ответа на запрос.
 * @author Artem Snopkov
 */
public class ResponseWaitTimeoutException extends IOException {

   ResponseWaitTimeoutException() {
  }

   ResponseWaitTimeoutException(String message) {
    super(message);
  }

   ResponseWaitTimeoutException(String message, Throwable cause) {
    super(message, cause);
  }

   ResponseWaitTimeoutException(Throwable cause) {
    super(cause);
  }
}
