package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public class SmscStatusManager {

  private final AdminContext adminContext;

  public SmscStatusManager(AdminContext adminContext) {
    this.adminContext = adminContext;
  }

  public int getSmscInstancesNumber() throws AdminException {
    return adminContext.getSmscManager().getSettings().getSmscInstancesCount();
  }

  public String getSmscOnlineHost(int instanceNumber) throws AdminException {
    return adminContext.getSmscManager().getSmscOnlineHost(instanceNumber);
  }

   public List<String> getSmscHosts(int instanceNumber) throws AdminException {
    return adminContext.getSmscManager().getSmscHosts(instanceNumber);
  }

  public SmscConfigurationStatus getMainConfigState(int instanceNumber) throws AdminException {
    return adminContext.getSmscManager().getStatusForSmscs().get(instanceNumber);
  }

  public SmscConfigurationStatus getRescheduleState(int instanceNumber) throws AdminException {
    return adminContext.getRescheduleManager().getStatusForSmscs().get(instanceNumber);
  }


  public void stopSmsc(int instanceNumber) throws AdminException {
    adminContext.getSmscManager().stopSmsc(instanceNumber);
  }

  public void startSmsc(int instanceNumber) throws AdminException {
    adminContext.getSmscManager().startSmsc(instanceNumber);
  }

  public void switchSmsc(int instanceNumber, String toHost) throws AdminException {
    adminContext.getSmscManager().switchSmsc(instanceNumber, toHost);
  }

}
