package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Artem Snopkov
 */
class MapLimitConfig implements ManagedConfigFile {

  static final int MAX_CONGESTION_LEVELS = 8;

  private MapLimitSettings settings;

  public MapLimitSettings getSettings() {
    return settings;
  }

  public void setSettings(MapLimitSettings settings) {
    this.settings = settings;
  }

  public void load(InputStream is) throws Exception {

    MapLimitSettings s = new MapLimitSettings();

    XmlConfig config = new XmlConfig();
    config.load(is);

    XmlConfigSection clevels = config.getSection("clevels");

    CongestionLevel clev[] = new CongestionLevel[MAX_CONGESTION_LEVELS];
    for (int i = 1; i <= MAX_CONGESTION_LEVELS; i++) {
      XmlConfigSection clevel = clevels.getSection("level" + i);
      CongestionLevel cl = new CongestionLevel(clevel.getInt("dialogsLimit"), clevel.getInt("failLowerLimit"), clevel.getInt("failUpperLimit"), clevel.getInt("okToLower"));
      clev[i-1] = cl;
    }

    s.setCongestionLevels(clev);


    s.setDlgLimitIn(config.getSection("dlglimit").getInt("in"));
    s.setDlgLimitInSri(config.getSection("dlglimit").getInt("insri"));
    s.setDlgLimitInUssd(config.getSection("dlglimit").getInt("niussd"));
    s.setDlgLimitOutSri(config.getSection("dlglimit").getInt("outsri"));
    s.setDlgLimitUssd(config.getSection("dlglimit").getInt("ussd"));
    s.setUssdNoSriCodes(config.getSection("ussd").getIntArray("no_sri_codes", ","));

    settings = s;
  }

  public void save(InputStream is, OutputStream os) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);

    XmlConfigSection clevels = config.getSection("clevels");

    for (int i = 1; i <= MAX_CONGESTION_LEVELS; i++) {
      CongestionLevel cl = this.settings.getCongestionLevels()[i-1];
      XmlConfigSection clevel=clevels.getSection("level"+i);
      clevel.setInt("dialogsLimit", cl.getDialogsLimit());
      clevel.setInt("failLowerLimit", cl.getFailLowerLimit());
      clevel.setInt("failUpperLimit", cl.getFailUpperLimit());
      clevel.setInt("okToLower", cl.getOkToLower());
    }

    config.getSection("dlglimit").setInt("in", settings.getDlgLimitIn());
    config.getSection("dlglimit").setInt("insri", settings.getDlgLimitInSri());
    config.getSection("dlglimit").setInt("niussd", settings.getDlgLimitInUssd());
    config.getSection("dlglimit").setInt("outsri", settings.getDlgLimitOutSri());
    config.getSection("dlglimit").setInt("ussd", settings.getDlgLimitUssd());
    config.getSection("ussd").setIntArray("no_sri_codes", settings.getUssdNoSriCodes(), ",");

    config.save(os);
  }
}
