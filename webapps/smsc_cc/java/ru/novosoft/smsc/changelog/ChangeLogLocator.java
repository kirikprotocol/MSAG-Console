package ru.novosoft.smsc.changelog;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;

/**
 * Вспомогательный класс, позволяющий найти ChangeLog по какой-либо части AdminContext, к которому привязан
 * данный ChangeLog.
 * @author Artem Snopkov
 */
public class ChangeLogLocator {

  /**
   * Возвращает ChangeLog по экземпляру SmscManager
   * @param manager экземпляр SmscManager
   * @return ChangeLog в который записываются все изменения в данном SmscManager
   */
  public static ChangeLog getInstance(SmscManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getSmscManager() == manager)
        return cl;
    }
    return null;
  }

  /**
   * Возвращает ChangeLog по экземпляру AliasManager
   * @param manager экземпляр AliasManager
   * @return ChangeLog в который записываются все изменения в данном AliasManager
   */
  public static ChangeLog getInstance(AliasManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getAliasManager() == manager)
        return cl;
    }
    return null;
  }

  /**
   * Возвращает ChangeLog по экземпляру ArchiveDaemonManager
   * @param manager экземпляр ArchiveDaemonManager
   * @return ChangeLog в который записываются все изменения в данном ArchiveDaemonManager
   */
  public static ChangeLog getInstance(ArchiveDaemonManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getArchiveDaemonManager() == manager)
        return cl;
    }
    return null;
  }
}
