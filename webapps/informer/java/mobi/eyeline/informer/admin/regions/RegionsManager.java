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
 *
 * @author Aleksandr Khalitov
 */
public class RegionsManager extends BaseManager<RegionsSettings> {

  public RegionsManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme, config, backup, fileSystem, new RegionsConfig());
  }

  /**
   * Проверяет, будет ли корректна конфигурация, состоящая из указанных регионов.
   * @param regions список регионов, входящих в конфигурацию
   * @throws AdminException если конфигурация будет некорректна
   */
  public void checkRegionsConfiguration(Collection<Region> regions) throws AdminException {
    RegionsSettings settings = new RegionsSettings();
    for (Region r : regions)
      settings.addRegion(r);
  }

  /**
   * Добавляет новый регион
   *
   * @param r регион
   * @throws AdminException ошибка сохранения
   */
  public void addRegion(Region r) throws AdminException {
    final Region region = r.cloneRegion();
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
   *
   * @param r регион
   * @throws AdminException ошибка сохранения
   */
  public void updateRegion(Region r) throws AdminException {
    final Region region = r.cloneRegion();
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
   *
   * @return список регионов
   */
  public List<Region> getRegions() {
    return readSettings(new SettingsReader<RegionsSettings, List<Region>>() {
      public List<Region> executeRead(RegionsSettings settings) {
        List<Region> result = new LinkedList<Region>();
        for (Region s : settings.getRegions()) {
          result.add(s.cloneRegion());
        }
        return result;
      }
    });
  }

  /**
   * Возвращает регион по id или null, если регион не найден
   *
   * @param id id региона
   * @return регион с указанным идентификатором или null, если регион не найден.
   */
  public Region getRegion(final Integer id) {
    return readSettings(new SettingsReader<RegionsSettings, Region>() {
      public Region executeRead(RegionsSettings settings) {
        Region region = settings.getRegion(id);
        return region == null ? null : region.cloneRegion();
      }
    });

  }

  /**
   * Удаляет регион
   *
   * @param regionId id региона
   * @throws AdminException ошибка сохранения
   */
  public void removeRegion(final Integer regionId) throws AdminException {
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
   * Установка скорости отправки сообщений в дефолтный регион
   *
   * @param defMaxPerSecond скорость отправки сообщений в дефолтный регион
   * @throws AdminException ошибка сохранения
   */
  public void setDefaultMaxPerSecond(final int defMaxPerSecond) throws AdminException {
    updateSettings(new SettingsWriter<RegionsSettings>() {
      public void changeSettings(RegionsSettings settings) throws AdminException {
        settings.setDefaultMaxPerSecond(defMaxPerSecond);
      }

      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.updateRegion(0);  // Default region has id=0
      }
    });
  }

  /**
   * Возвращает скорость отправки сообщений в дефолтный регион
   *
   * @return скорость отправки сообщений в дефолтный регион
   */
  public int getDefaultMaxPerSecond() {
    return readSettings(new SettingsReader<RegionsSettings, Integer>() {
      public Integer executeRead(RegionsSettings settings) {
        return settings.getDefaultMaxPerSecond();
      }
    });
  }


  /**
   * Возвращает регион по маске или null, если регион не найден
   *
   * @param address маска
   * @return регион или null, если регион не найден.
   */
  public Region getRegion(final Address address) {
    return readSettings(new SettingsReader<RegionsSettings, Region>() {
      public Region executeRead(RegionsSettings settings) {
        Region region = settings.getRegionByAddress(address);
        return region == null ? null : region.cloneRegion();
      }
    });
  }

  /**
   * Возвращает список регионов по названию СМСЦ
   *
   * @param smsc название СМСЦ
   * @return список регионов
   */
  public Collection<Region> getRegionsBySmsc(final String smsc) {
    return readSettings(new SettingsReader<RegionsSettings, Collection<Region>>() {
      public Collection<Region> executeRead(RegionsSettings settings) {
        Collection<Region> result = new LinkedList<Region>();
        for (Region r : settings.getRegionsBySmsc(smsc)) {
          result.add(r.cloneRegion());
        }

        return result;
      }
    });
  }

  /**
   * Проверяет наличие в конфигурации региона с указанным именем
   * @param name имя региона
   * @return true, если регион присутствует, иначе - false
   */
  public boolean containsRegionWithName(final String name) {
    if (name == null)
      return false;
    return readSettings(new SettingsReader<RegionsSettings, Boolean>() {
      public Boolean executeRead(RegionsSettings settings) {
        for (Region r : settings.getRegions())
          if (r.getName().equals(name))
            return true;
        return false;
      }
    });
  }

}
