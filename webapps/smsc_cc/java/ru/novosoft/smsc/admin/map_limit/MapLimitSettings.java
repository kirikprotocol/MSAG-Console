package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;
import java.util.Arrays;

/**
 * @author Artem Snopkov
 */
public class MapLimitSettings implements Serializable {

  public static final int MAX_CONGESTION_LEVELS = 8;

  private static final ValidationHelper vh = new ValidationHelper(MapLimitSettings.class);

  private int dlgLimitIn;
  private int dlgLimitInSri;
  private int dlgLimitInUssd;
  private int dlgLimitOutSri;
  private int dlgLimitUssd;
  private int[] ussdNoSriCodes;
  private CongestionLevel clevels[] = new CongestionLevel[MAX_CONGESTION_LEVELS];

  MapLimitSettings() {
  }

  MapLimitSettings(MapLimitSettings c) {
    this.dlgLimitIn = c.dlgLimitIn;
    this.dlgLimitInSri = c.dlgLimitInSri;
    this.dlgLimitInUssd = c.dlgLimitInUssd;
    this.dlgLimitOutSri = c.dlgLimitOutSri;
    this.dlgLimitUssd = c.dlgLimitUssd;

    this.ussdNoSriCodes = new int[c.ussdNoSriCodes.length];
    System.arraycopy(c.ussdNoSriCodes, 0, this.ussdNoSriCodes, 0, c.ussdNoSriCodes.length);

    this.clevels = new CongestionLevel[MAX_CONGESTION_LEVELS];
    for(int i=0;i<MAX_CONGESTION_LEVELS;i++) {
      this.clevels[i] = new CongestionLevel(c.clevels[i]);
    }
  }

  public int getDlgLimitIn() {
    return dlgLimitIn;
  }

  public void setDlgLimitIn(int dlgLimitIn) throws AdminException {
    vh.checkPositive("dlgLimitIn", dlgLimitIn);
    this.dlgLimitIn = dlgLimitIn;
  }

  public int getDlgLimitInSri() {
    return dlgLimitInSri;
  }

  public void setDlgLimitInSri(int dlgLimitInSri) throws AdminException {
    vh.checkPositive("dlgLimitInSri", dlgLimitInSri);
    this.dlgLimitInSri = dlgLimitInSri;
  }

  public int getDlgLimitInUssd() {
    return dlgLimitInUssd;
  }

  public void setDlgLimitInUssd(int dlgLimitInUssd) throws AdminException {
    vh.checkPositive("dlgLimitInUssd", dlgLimitInUssd);
    this.dlgLimitInUssd = dlgLimitInUssd;
  }

  public int getDlgLimitOutSri() {
    return dlgLimitOutSri;
  }

  public void setDlgLimitOutSri(int dlgLimitOutSri) throws AdminException {
    vh.checkPositive("dlgLimitOutSri", dlgLimitOutSri);
    this.dlgLimitOutSri = dlgLimitOutSri;
  }

  public int getDlgLimitUssd() {
    return dlgLimitUssd;
  }

  public void setDlgLimitUssd(int dlgLimitUssd) throws AdminException {
    vh.checkPositive("dlgLimitUssd", dlgLimitUssd);
    this.dlgLimitUssd = dlgLimitUssd;
  }

  public int[] getUssdNoSriCodes() {
    return ussdNoSriCodes;
  }

  public void setUssdNoSriCodes(int[] ussdNoSriCodes) throws AdminException {
    vh.checkPositive("ussdNoSriCodes", ussdNoSriCodes);
    this.ussdNoSriCodes = ussdNoSriCodes;
  }

  public CongestionLevel[] getCongestionLevels() {
    return clevels;
  }

  public void setCongestionLevels(CongestionLevel[] clevels) throws AdminException {
    vh.checkNoNulls("congestionLevels", Arrays.asList(clevels));
    vh.checkSize("congestionLevels", clevels, MAX_CONGESTION_LEVELS);
    this.clevels = clevels;
  }

  public MapLimitSettings cloneSettings() {
    return new MapLimitSettings(this);
  }

}
