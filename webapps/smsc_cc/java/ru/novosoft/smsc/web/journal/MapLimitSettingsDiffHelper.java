package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;

/**
 * @author Artem Snopkov
 */
class MapLimitSettingsDiffHelper extends DiffHelper {


  public MapLimitSettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, MapLimitSettings oldSettings, MapLimitSettings newSettings, String user) {
    // todo
  }
}
