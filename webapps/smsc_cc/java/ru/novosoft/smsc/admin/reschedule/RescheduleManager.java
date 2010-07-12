package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Collection;
import java.util.List;
import java.util.regex.Pattern;

/**
 * @author Artem Snopkov
 */
public class RescheduleManager {

  private RescheduleConfig config;
  private final ClusterController cc;
  private final File configFile;
  private final File backupDir;
  private final FileSystem fileSystem;
  private boolean changed;


  public RescheduleManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.configFile = configFile;
    this.backupDir = backupDir;
    this.fileSystem = fs;
    this.cc = cc;
    this.config = createConfig();
    reset();
  }

  protected RescheduleConfig createConfig() {
    return new RescheduleConfig(configFile, backupDir, fileSystem);
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

  public void apply() throws AdminException {
    this.config.save();
    cc.applyReschedule();
    changed = false;
  }

  public void reset() throws AdminException {

    RescheduleConfig oldConfig = createConfig();
    oldConfig.load();

    this.config = oldConfig;
    changed = false;
  }
}
