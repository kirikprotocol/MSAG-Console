package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.ValidationHelper;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * Дополнительные настройки пользователя
 * @author Aleksandr Khalitov
 */
public class UserPreferences implements Serializable{

  private Locale locale;

  private Map<String, String> unknown = new HashMap<String, String>();

  private final ValidationHelper vh = new ValidationHelper(UserPreferences.class);

  //topMon

  private int topMonGraphHead = 20;
  private int topMonGraphScale = 3;
  private int topMonMaxSpeed = 50;
  private int topMonGraphHigrid = 10;
  private int topMonGraphGrid = 2;

  //perfMon

  private int perfMonVMinuteGrid = 6;
  private int perfMonVLightGrid = 4;
  private int perfMonBlock = 8;
  private int perfMonPixPerSecond = 4;
  private int perfMonScale = 80;

  public UserPreferences() {
  }

  public UserPreferences(UserPreferences prefs) {
    this.locale = prefs.locale == null ? null : new Locale(prefs.locale.getLanguage());
    this.topMonGraphGrid = prefs.topMonGraphGrid;
    this.topMonGraphHead = prefs.topMonGraphHead;
    this.topMonMaxSpeed = prefs.topMonMaxSpeed;
    this.topMonGraphHigrid = prefs.topMonGraphHigrid;
    this.topMonGraphScale = prefs.topMonGraphScale;
    this.perfMonVMinuteGrid = prefs.perfMonVMinuteGrid;
    this.perfMonVLightGrid = prefs.perfMonVLightGrid;
    this.perfMonBlock = prefs.perfMonBlock;
    this.perfMonPixPerSecond = prefs.perfMonPixPerSecond;
    this.perfMonScale = prefs.perfMonScale;
    this.unknown = unknown != null ? new HashMap<String, String>(prefs.unknown) : null;
  }

  public Locale getLocale() {
    return locale;
  }

  public void setLocale(Locale locale) throws AdminException {
    vh.checkNotNull("locale", locale);
    this.locale = locale;
  }

  public Map<String, String> getUnknown() {
    return unknown;
  }

  public void setUnknown(Map<String, String> unknown) {
    this.unknown = unknown;
  }

  public int getTopMonGraphHead() {
    return topMonGraphHead;
  }

  public void setTopMonGraphHead(int topMonGraphHead) throws AdminException {
    vh.checkPositive("topMonGraphHead", topMonGraphHead);
    this.topMonGraphHead = topMonGraphHead;
  }

  public int getTopMonGraphScale() {
    return topMonGraphScale;
  }

  public void setTopMonGraphScale(int topMonGraphScale) throws AdminException {
    vh.checkPositive("topMonGraphScale", topMonGraphScale);
    this.topMonGraphScale = topMonGraphScale;
  }

  public int getTopMonMaxSpeed() {
    return topMonMaxSpeed;
  }

  public void setTopMonMaxSpeed(int topMonMaxSpeed) throws AdminException {
    vh.checkPositive("topMonMaxSpeed", topMonMaxSpeed);
    this.topMonMaxSpeed = topMonMaxSpeed;
  }

  public int getTopMonGraphHigrid() {
    return topMonGraphHigrid;
  }

  public void setTopMonGraphHigrid(int topMonGraphHigrid) throws AdminException {
    vh.checkPositive("topMonGraphHigrid", topMonGraphHigrid);
    this.topMonGraphHigrid = topMonGraphHigrid;
  }

  public int getTopMonGraphGrid() {
    return topMonGraphGrid;
  }

  public void setTopMonGraphGrid(int topMonGraphGrid) throws AdminException {
    vh.checkPositive("topMonGraphGrid", topMonGraphGrid);
    this.topMonGraphGrid = topMonGraphGrid;
  }

  public int getPerfMonVMinuteGrid() {
    return perfMonVMinuteGrid;
  }

  public void setPerfMonVMinuteGrid(int perfMonVMinuteGrid) throws AdminException {
    vh.checkPositive("perfMonVMinuteGrid", perfMonVMinuteGrid);
    this.perfMonVMinuteGrid = perfMonVMinuteGrid;
  }

  public int getPerfMonVLightGrid() {
    return perfMonVLightGrid;
  }

  public void setPerfMonVLightGrid(int perfMonVLightGrid) throws AdminException {
    vh.checkPositive("perfMonVLightGrid", perfMonVLightGrid);
    this.perfMonVLightGrid = perfMonVLightGrid;
  }

  public int getPerfMonBlock() {
    return perfMonBlock;
  }

  public void setPerfMonBlock(int perfMonBlock) throws AdminException {
    vh.checkPositive("perfMonBlock", perfMonBlock);
    this.perfMonBlock = perfMonBlock;
  }

  public int getPerfMonPixPerSecond() {
    return perfMonPixPerSecond;
  }

  public void setPerfMonPixPerSecond(int perfMonPixPerSecond) throws AdminException {
    vh.checkPositive("perfMonPixPerSecond", perfMonPixPerSecond);
    this.perfMonPixPerSecond = perfMonPixPerSecond;
  }

  public int getPerfMonScale() {
    return perfMonScale;
  }

  public void setPerfMonScale(int perfMonScale) throws AdminException {
    vh.checkPositive("perfMonScale", perfMonScale);
    this.perfMonScale = perfMonScale;
  }

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("UserPreferences");
    sb.append("{locale=").append(locale);
    sb.append(", topMonGraphHead=").append(topMonGraphHead);
    sb.append(", topMonGraphScale=").append(topMonGraphScale);
    sb.append(", topMonMaxSpeed=").append(topMonMaxSpeed);
    sb.append(", topMonGraphHigrid=").append(topMonGraphHigrid);
    sb.append(", topMonGraphGrid=").append(topMonGraphGrid);
    sb.append(", perfMonVMinuteGrid=").append(perfMonVMinuteGrid);
    sb.append(", perfMonVLightGrid=").append(perfMonVLightGrid);
    sb.append(", perfMonBlock=").append(perfMonBlock);
    sb.append(", perfMonPixPerSecond=").append(perfMonPixPerSecond);
    sb.append(", perfMonScale=").append(perfMonScale);
    sb.append('}');
    return sb.toString();
  }

  @SuppressWarnings({"RedundantIfStatement"})
  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    UserPreferences that = (UserPreferences) o;

    if (perfMonBlock != that.perfMonBlock) return false;
    if (perfMonPixPerSecond != that.perfMonPixPerSecond) return false;
    if (perfMonScale != that.perfMonScale) return false;
    if (perfMonVLightGrid != that.perfMonVLightGrid) return false;
    if (perfMonVMinuteGrid != that.perfMonVMinuteGrid) return false;
    if (topMonGraphGrid != that.topMonGraphGrid) return false;
    if (topMonGraphHead != that.topMonGraphHead) return false;
    if (topMonGraphHigrid != that.topMonGraphHigrid) return false;
    if (topMonGraphScale != that.topMonGraphScale) return false;
    if (topMonMaxSpeed != that.topMonMaxSpeed) return false;
    if (locale != null ? !locale.equals(that.locale) : that.locale != null) return false;

    return true;
  }
}
