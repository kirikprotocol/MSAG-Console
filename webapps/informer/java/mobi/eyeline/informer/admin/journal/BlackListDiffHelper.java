package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Логирует изменения в запрещенных номерах
 * @author Aleksandr Khalitov
 */
class BlackListDiffHelper extends DiffHelper{

  BlackListDiffHelper() {
    super(Subject.BLACKLIST);
  }

  void logAddBlacklist(Journal j, String address, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.ADD, subject, user, "blacklist_added", address);
  }

  void logRemoveBlacklist(Journal j, String address, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.REMOVE, subject, user, "blacklist_removed", address);
  }

}
