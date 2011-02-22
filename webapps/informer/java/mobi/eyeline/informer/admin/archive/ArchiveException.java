package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class ArchiveException extends AdminException{

  ArchiveException(String key, String... args) {
    super(key, args);
  }
}
