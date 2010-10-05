package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class RegionSettings implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(RegionSettings.class);

  private Map<Integer, Region> regions;
  private int defaultBandwidth;
  private String defaultEmail;
  private int maxRegionId;

  RegionSettings(Collection<Region> regions, int defaultBandwidth, String defaultEmail) {
    this.regions = new HashMap<Integer, Region>();
    this.maxRegionId = -1;
    for (Region r : regions) {
      this.regions.put(r.getId(), r);
      maxRegionId = Math.max(maxRegionId, r.getId());
    }
    this.defaultBandwidth = defaultBandwidth;
    this.defaultEmail = defaultEmail;
  }

  RegionSettings(RegionSettings copy) {
    this.defaultBandwidth = copy.defaultBandwidth;
    this.defaultEmail = copy.defaultEmail;
    this.maxRegionId = copy.maxRegionId;
    this.regions = new HashMap<Integer, Region>();
    for (Map.Entry<Integer, Region> e : copy.regions.entrySet())
      this.regions.put(e.getKey(), new Region(e.getValue()));
  }

  /**
   * Создает новый регион, добавляет его в список регионов и возвращает копию.
   * @param name имя нового региона
   * @param bandwidth пропускная способность
   * @param email email
   * @param subjects список субъектов для данного региона
   * @return копию нового региона
   * @throws ru.novosoft.smsc.admin.AdminException если параметры региона некорректны
   */
  public Region createRegion(String name, int bandwidth, String email, Collection<String> subjects) throws AdminException {
    Region newRegion = new Region(++maxRegionId, name);
    newRegion.setEmail(email);
    newRegion.setBandwidth(bandwidth);
    newRegion.setSubjects(subjects);
    regions.put(newRegion.getId(), newRegion);
    return new Region(newRegion);
  }

  /**
   * Удаляет регион по его идентификатору
   * @param id идентификатор региона
   * @return true, если регион с таким идентификатором существует. Иначе, возвращается false.
   */
  public boolean removeRegion(int id) {
    return regions.remove(id) != null;
  }

  /**
   * Обновляет информацию о регионе
   * @param region новые сведения о регионе
   */
  public void updateRegion(Region region) {
    regions.put(region.getId(), new Region(region));
  }

  /**
   * Возвращает копию региона по его идентификатору или null, если региона не существует
   * @param id идентификатор региона
   * @return копию региона по его идентификатору или null, если региона не существует
   */
  public Region getRegion(int id) {
    if (!regions.containsKey(id))
      return null;
    return new Region(regions.get(id));
  }

  /**
   * Возвращает копию списка регионов
   * @return копию списка регионов
   */
  public Collection<Region> getRegions() {
    List<Region> result = new ArrayList<Region>(regions.size());
    for (Region r : regions.values())
      result.add(new Region(r));
    return result;
  }

  /**
   * Возвращает значение параметра defaultBandwidth
   * @return значение параметра defaultBandwidth
   */
  public int getDefaultBandwidth() {
    return defaultBandwidth;
  }

  /**
   * Устанавливает значение параметра defaultBandwidth
   * @param defaultBandwidth новое значение параметра defaultBandwidth
   */
  public void setDefaultBandwidth(int defaultBandwidth) throws AdminException {
    vh.checkGreaterOrEqualsTo("defaultBandwidth", defaultBandwidth, 0);
    this.defaultBandwidth = defaultBandwidth;
  }

  /**
   * Возвращает значение параметра defaultEmail
   * @return значение параметра defaultEmail
   */
  public String getDefaultEmail() {
    return defaultEmail;
  }

  /**
   * Устанавливает значение параметра defaultEmail
   * @param defaultEmail новое значение параметра defaultEmail
   */
  public void setDefaultEmail(String defaultEmail) {
    this.defaultEmail = defaultEmail;
  }

  public RegionSettings cloneSettings() {
    return new RegionSettings(this);
  }
}
