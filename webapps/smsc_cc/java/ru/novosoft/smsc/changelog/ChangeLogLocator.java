package ru.novosoft.smsc.changelog;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;

/**
 * Вспомогательный класс, позволяющий найти ChangeLog по какой-либо части AdminContext, к которому привязан
 * данный ChangeLog.
 * @author Artem Snopkov
 */
public class ChangeLogLocator {

  /**
   * Возвращает ChangeLog по экземпляру SmscManager
   * @param manager экземпляру SmscManager
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
   * @param manager экземпляру AliasManager
   * @return ChangeLog в который записываются все изменения в данном AliasManager
   */
  public static ChangeLog getInstance(AliasManager manager) {
    for (ChangeLog cl : ChangeLog.instances) {
      if (cl.getAdminContext().getAliasManager() == manager)
        return cl;
    }
    return null;
  }
}
