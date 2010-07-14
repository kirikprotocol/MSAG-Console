package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class RescheduleManager extends ConfigFileManager<RescheduleConfig> {

  private final ClusterController cc;

  public RescheduleManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    super(configFile, backupDir, fs);
    this.cc = cc;
    reset();
  }

  public void setScheduleLimit(int limit) {
    config.setRescheduleLimit(limit);
    changed = true;
  }

  public int getScheduleLimit() {
    return config.getRescheduleLimit();
  }

  public String getDefaultReschedule() {
    return config.getDefaultReschedule();
  }

  public void setDefaultReschedule(String intervals) {
    config.setDefaultReschedule(intervals);
    changed = true;
  }

  public Collection<Reschedule> getReschedules() {
    return config.getReschedules();
  }

  public void setReschedules(Collection<Reschedule> reschedules) {
    config.setReschedules(reschedules);
    changed = true;
  }

  public boolean isChanged() {
    return changed;
  }

  @Override
  protected RescheduleConfig newConfigFile() {
    return new RescheduleConfig();
  }

  @Override
  protected void afterApply() throws AdminException {
    cc.applyReschedule();
  }
}
