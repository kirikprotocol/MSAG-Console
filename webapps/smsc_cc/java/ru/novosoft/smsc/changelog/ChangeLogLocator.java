package ru.novosoft.smsc.changelog;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.msc.MscManager;
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

  /**
   * Возвращает ChangeLog по экземпляру ClosedGroupManager
   * @param manager экземпляр ClosedGroupManager
   * @return ChangeLog в который записываются все изменения в данном ClosedGroupManager
   */
  public static ChangeLog getInstance(ClosedGroupManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getClosedGroupManager() == manager)
        return cl;
    }
    return null;
  }

  /**
   * Возвращает ChangeLog по экземпляру FraudManager
   * @param manager экземпляр FraudManager
   * @return ChangeLog в который записываются все изменения в данном FraudManager
   */
  public static ChangeLog getInstance(FraudManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getFraudManager() == manager)
        return cl;
    }
    return null;
  }

  /**
   * Возвращает ChangeLog по экземпляру MapLimitManager
   * @param manager экземпляр MapLimitManager
   * @return ChangeLog в который записываются все изменения в данном MapLimitManager
   */
  public static ChangeLog getInstance(MapLimitManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getMapLimitManager() == manager)
        return cl;
    }
    return null;
  }

  /**
   * Возвращает ChangeLog по экземпляру MscManager
   * @param manager экземпляр MscManager
   * @return ChangeLog в который записываются все изменения в данном MscManager
   */
  public static ChangeLog getInstance(MscManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getMscManager() == manager)
        return cl;
    }
    return null;
  }
}
