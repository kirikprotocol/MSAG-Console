package ru.novosoft.smsc.web.config.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.MCISmeManager;
import ru.novosoft.smsc.admin.mcisme.MCISmeSettings;
import ru.novosoft.smsc.admin.mcisme.MessageStatistics;
import ru.novosoft.smsc.admin.mcisme.StatFilter;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class WMCISmeManager extends BaseSettingsManager<MCISmeSettings> implements MCISmeManager{


  private final Journal j;
  private final MCISmeManager wrapped;

  public WMCISmeManager(MCISmeManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(MCISmeSettings settings) throws AdminException {
    wrapped.updateSettings(settings);
  }

  @Override
  protected void logChanges(MCISmeSettings oldSettings, MCISmeSettings newSettings) {

  //todo
  }

  public String getSmeOnlineHost() throws AdminException {
    return wrapped.getSmeOnlineHost();
  }

  public void switchSme(String toHost) throws AdminException {
    wrapped.switchSme(toHost);
  }

  public void startSme() throws AdminException {
    wrapped.startSme();
  }

  public void stopSme() throws AdminException {
    wrapped.stopSme();
  }

  public List<String> getSmeHosts() throws AdminException {
    return wrapped.getSmeHosts();
  }

  public MCISmeSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public MessageStatistics getStatistics(StatFilter filter) throws AdminException {
    return wrapped.getStatistics(filter);
  }

  public MCISmeSettings cloneSettings(MCISmeSettings settings) {
    return new MCISmeSettings(settings);
  }
}
