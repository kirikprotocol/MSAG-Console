package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 17:56:17
 */
class RestrictionsManager extends BaseManager<RestrictionSettings> {

  public RestrictionsManager(File config, File backup, FileSystem fileSystem) throws InitException {
    super(null, config, backup, fileSystem, new RestrictionsConfig());
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

  public List<Restriction> getRestrictions() {
    return readSettings(new SettingsReader<RestrictionSettings, List<Restriction>>() {
      public List<Restriction> executeRead(RestrictionSettings settings) {
        return settings.getRestrictions(new RestrictionsFilter());
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

}
