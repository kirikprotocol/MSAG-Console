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

  private int dlgLimitIn;
  private int dlgLimitInSri;
  private int dlgLimitInUssd;
  private int dlgLimitOutSri;
  private int dlgLimitUssd;
  private int[] ussdNoSriCodes;
  private CongestionLevel clevels[] = new CongestionLevel[MAX_CONGESTION_LEVELS];

  public int getDlgLimitIn() {
    return dlgLimitIn;
  }

  public void setDlgLimitIn(int dlgLimitIn) {
    if (dlgLimitIn < 0)
      throw new IllegalArgumentException();
    this.dlgLimitIn = dlgLimitIn;
  }

  public int getDlgLimitInSri() {
    return dlgLimitInSri;
  }

  public void setDlgLimitInSri(int dlgLimitInSri) {
    if (dlgLimitInSri < 0)
      throw new IllegalArgumentException();
    this.dlgLimitInSri = dlgLimitInSri;
  }

  public int getDlgLimitInUssd() {
    return dlgLimitInUssd;   
  }

  public void setDlgLimitInUssd(int dlgLimitInUssd) {
    if (dlgLimitInUssd < 0)
      throw new IllegalArgumentException();
    this.dlgLimitInUssd = dlgLimitInUssd;
  }

  public int getDlgLimitOutSri() {
    return dlgLimitOutSri;
  }

  public void setDlgLimitOutSri(int dlgLimitOutSri) {
    if (dlgLimitOutSri < 0)
      throw new IllegalArgumentException();
    this.dlgLimitOutSri = dlgLimitOutSri;
  }

  public int getDlgLimitUssd() {
    return dlgLimitUssd;
  }

  public void setDlgLimitUssd(int dlgLimitUssd) {
    if (dlgLimitUssd < 0)
      throw new IllegalArgumentException();
    this.dlgLimitUssd = dlgLimitUssd;
  }

  public int[] getUssdNoSriCodes() {
    return ussdNoSriCodes;
  }

  public void setUssdNoSriCodes(int[] ussdNoSriCodes) {
    if (ussdNoSriCodes == null)
      throw  new IllegalArgumentException();
    int[] copy = new int[ussdNoSriCodes.length];
    System.arraycopy(ussdNoSriCodes, 0, copy, 0, ussdNoSriCodes.length);
    this.ussdNoSriCodes = copy;
  }

  public CongestionLevel[] getClevels() {
    return clevels;
  }

  public void setClevels(CongestionLevel[] clevels) {
    if (clevels == null)
      throw new IllegalArgumentException();
    if (clevels.length != MapLimitConfig.MAX_CONGESTION_LEVELS)
      throw new IllegalArgumentException();
    for (int i=0; i<MAX_CONGESTION_LEVELS; i++)
      if (clevels[i] == null)
        throw new IllegalArgumentException();

    CongestionLevel[] copy = new CongestionLevel[MAX_CONGESTION_LEVELS];
    System.arraycopy(clevels, 0, copy, 0, MAX_CONGESTION_LEVELS);
    this.clevels = copy;
  }

  public void load(InputStream is) throws Exception {

    XmlConfig config = new XmlConfig();
    config.load(is);

    XmlConfigSection clevels = config.getSection("clevels");

    for (int i = 1; i <= MAX_CONGESTION_LEVELS; i++) {
      XmlConfigSection clevel = clevels.getSection("level" + i);
      CongestionLevel cl = new CongestionLevel(clevel.getInt("dialogsLimit"), clevel.getInt("failLowerLimit"), clevel.getInt("failUpperLimit"), clevel.getInt("okToLower"));
      this.clevels[i-1] = cl;
    }

    dlgLimitIn = config.getSection("dlglimit").getInt("in");
    dlgLimitInSri = config.getSection("dlglimit").getInt("insri");
    dlgLimitInUssd = config.getSection("dlglimit").getInt("niussd");
    dlgLimitOutSri = config.getSection("dlglimit").getInt("outsri");
    dlgLimitUssd = config.getSection("dlglimit").getInt("ussd");
    ussdNoSriCodes = config.getSection("ussd").getIntArray("no_sri_codes", ",");
  }

  public void save(InputStream is, OutputStream os) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);

    XmlConfigSection clevels = config.getSection("clevels");

    for (int i = 1; i <= MAX_CONGESTION_LEVELS; i++) {
      CongestionLevel cl = this.clevels[i-1];
      XmlConfigSection clevel=clevels.getSection("level"+i);
      clevel.setInt("dialogsLimit", cl.getDialogsLimit());
      clevel.setInt("failLowerLimit", cl.getFailLowerLimit());
      clevel.setInt("failUpperLimit", cl.getFailUpperLimit());
      clevel.setInt("okToLower", cl.getOkToLower());
    }

    config.getSection("dlglimit").setInt("in", dlgLimitIn);
    config.getSection("dlglimit").setInt("insri", dlgLimitInSri);
    config.getSection("dlglimit").setInt("niussd", dlgLimitInUssd);
    config.getSection("dlglimit").setInt("outsri", dlgLimitOutSri);
    config.getSection("dlglimit").setInt("ussd", dlgLimitUssd);
    config.getSection("ussd").setIntArray("no_sri_codes", ussdNoSriCodes, ",");

    config.save(os);
  }
}
