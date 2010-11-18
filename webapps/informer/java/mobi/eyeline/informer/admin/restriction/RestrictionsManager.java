package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.Date;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 17:56:17
 */
public class RestrictionsManager extends BaseManager<RestrictionSettings> {

  public RestrictionsManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme, config, backup, fileSystem, new RestrictionsConfig());
  }

  public Restriction getRestriction(final int id) {
    return readSettings(new SettingsReader<RestrictionSettings, Restriction>() {
      public Restriction executeRead(RestrictionSettings settings) {
        return settings.getRestriction(id);
      }
    });
  }

  public List<Restriction> getRestrictions(final RestrictionsFilter filter) {
    return readSettings(new SettingsReader<RestrictionSettings, List<Restriction>>() {
      public List<Restriction> executeRead(RestrictionSettings settings) {
        return settings.getRestrictions(filter);
      }
    });
  }

  public void addRestriction(final Restriction r) throws AdminException {
    updateSettings(new SettingsWriter<RestrictionSettings>() {
      public void changeSettings(RestrictionSettings settings) throws AdminException {
        settings.addRestriction(r);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //nothing to do
      }
    });
  }

  public void updateRestriction(final Restriction r) throws AdminException {
    updateSettings(new SettingsWriter<RestrictionSettings>() {
      public void changeSettings(RestrictionSettings settings) throws AdminException {
        settings.updateRestriction(r);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //nothing to do
      }
    });
  }

  public void deleteRestriction(final int id) throws AdminException {
    updateSettings(new SettingsWriter<RestrictionSettings>() {
      public void changeSettings(RestrictionSettings settings) throws AdminException {
        settings.deleteRestriction(id);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        //nothing to do
      }
    });
  }

  public boolean hasActiveRestriction(String userId) {
    RestrictionsFilter filter = new RestrictionsFilter();
    Date startDate = new Date();
    filter.setStartDate(startDate);
    filter.setEndDate(startDate);
    filter.setUserId(userId);
    return getRestrictions(filter).size() > 0;
  }

}
