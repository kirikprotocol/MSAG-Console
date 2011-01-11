package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RestrictionProvider {

  private final RestrictionDaemon daemon;
  private final RestrictionsManager manager;

  public RestrictionProvider(RestrictionContext context, File config, File backup, FileSystem fileSystem) throws InitException, AdminException {
    this.manager = new RestrictionsManager(config, backup, fileSystem);
    this.daemon = new RestrictionDaemon(context, this.manager);
    this.daemon.start();
  }

  public void shutdown() {
    try {
      this.daemon.stop();
    } catch (AdminException ignored) {
    }
  }

  public Restriction getRestriction(final int id) {
    return manager.getRestriction(id);
  }

  public List<Restriction> getRestrictions(final RestrictionsFilter filter) {
    return manager.getRestrictions(filter);
  }

  public void addRestriction(final Restriction r) throws AdminException {
    manager.addRestriction(r);
    daemon.rebuildSchedule();
  }

  public void updateRestriction(final Restriction r) throws AdminException {
    manager.updateRestriction(r);
    daemon.rebuildSchedule();
  }

  public void deleteRestriction(final int id) throws AdminException {
    manager.deleteRestriction(id);
    daemon.rebuildSchedule();
  }

  public void reschedule() {
    daemon.rebuildSchedule();
  }
}
