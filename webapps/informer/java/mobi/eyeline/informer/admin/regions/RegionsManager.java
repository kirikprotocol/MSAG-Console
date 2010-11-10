package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;
import mobi.eyeline.informer.util.Address;

import java.io.File;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

/**
 * Управление конфигурацией регионов
 * @author Aleksandr Khalitov
 */
public class RegionsManager extends BaseManager<RegionsSettings>{

  public RegionsManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme,  config, backup, fileSystem,new RegionsConfig());
  }

  /**
   * Добавляет новый регион
   * @param region регион
   * @throws AdminException ошибка сохранения
   */
  public void addRegion(final Region region) throws AdminException{
    updateSettings(new SettingsWriter<RegionsSettings>() {
      public void changeSettings(RegionsSettings settings) throws AdminException {
        settings.addRegion(region);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.addRegion(region.getRegionId());
      }
    });
  }

  /**
   * Обновляет существующий регион
   * @param region регион
   * @throws AdminException ошибка сохранения
   */
  public void updateRegion(final Region region) throws AdminException{
    updateSettings(new SettingsWriter<RegionsSettings>() {
      public void changeSettings(RegionsSettings settings) throws AdminException {
        settings.updateRegion(region);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.updateRegion(region.getRegionId());
      }
    });
  }

  /**
   * Возвращает список регионов
   * @return список регионов
   */
  public List<Region> getRegions()  {
    return readSettings(new SettingsReader<RegionsSettings, List<Region>>(){
      public List<Region> executeRead(RegionsSettings settings)  {
        List<Region> result = new LinkedList<Region>();
        for(Region s : settings.getRegions()) {
          result.add(s.cloneRegion());
        }
        return result;
      }
    });
  }

  /**
   * Возвращает регион по id
   * @param id id региона
   * @return регион
   */
  public Region getRegion(final Integer id)  {
    return readSettings(new SettingsReader<RegionsSettings, Region>(){
      public Region executeRead(RegionsSettings settings)  {
        Region region = settings.getRegion(id);
        return region == null ? null : region.cloneRegion();
      }
    });

  }

  /**
   * Удаляет регион
   * @param regionId id региона
   * @throws AdminException ошибка сохранения
   */
  public void removeRegion(final Integer regionId) throws AdminException{
    updateSettings(new SettingsWriter<RegionsSettings>() {
      public void changeSettings(RegionsSettings settings) throws AdminException {
        settings.removeRegion(regionId);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.removeRegion(regionId);
      }
    });
  }

  /**
   * Установка максимального кол-ва смс по умолчанию
   * @param defMaxPerSecond максимального кол-ва смс по умолчанию
   * @throws AdminException ошибка сохранения
   */
  public void setDefaultMaxPerSecond(final int defMaxPerSecond) throws AdminException{
    updateSettings(new SettingsWriter<RegionsSettings>() {
      public void changeSettings(RegionsSettings settings) throws AdminException {
        settings.setDefaultMaxPerSecond(defMaxPerSecond);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {}
    });
  }
  /**
   * Возвращает максимальное кол-во смс по умолчанию
   * @return максимального кол-ва смс по умолчанию
   */
  public int getDefaultMaxPerSecond(){
    return readSettings(new SettingsReader<RegionsSettings, Integer>(){
      public Integer executeRead(RegionsSettings settings) {
        return settings.getDefaultMaxPerSecond();
      }
    });
  }

  /**
   * Возвращает регион по маске
   * @param address маска
   * @return регион
   */
  public Region getRegion(final Address address) {
    return readSettings(new SettingsReader<RegionsSettings, Region>(){
      public Region executeRead(RegionsSettings settings)  {
        Region region = settings.getRegionByAddress(address);
        return region == null ? null : region.cloneRegion();
      }
    });
  }

  /**
   * Возвращает список регионов по названию СМСЦ
   * @param smsc название СМСЦ
   * @return список регионов
   */
  public Collection<Region> getRegionsBySmsc(final String smsc)  {
    return readSettings(new SettingsReader<RegionsSettings, Collection<Region>>(){
      public Collection<Region> executeRead(RegionsSettings settings)  {
        Collection<Region> result = new LinkedList<Region>();
        for(Region r : settings.getRegionsBySmsc(smsc)) {
          result.add(r.cloneRegion());
        }

        return result;
      }
    });
  }

}
