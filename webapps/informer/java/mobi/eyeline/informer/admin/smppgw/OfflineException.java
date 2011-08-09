package mobi.eyeline.informer.admin.smppgw;

/**
 * author: Aleksandr Khalitov
 */
class OfflineException extends SmppGWException {

  OfflineException(String key, String... args) {
    super(key, args);
  }
}
