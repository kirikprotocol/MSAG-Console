package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.LinkedList;
import java.util.List;

/**
 * Управление СМС-Центрами
 * @author Aleksandr Khalitov
 */
public class SmscManager extends BaseManager<SmscSettings>{


  public SmscManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException{
    super(infosme,  config, backup, fileSystem,new SmscConfig());

  }

  /**
   * Добавляет новый СМСЦ
   * @param smsc СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void addSmsc(final Smsc smsc) throws AdminException{
     updateSettings(new SettingsWriter<SmscSettings>() {
      public void changeSettings(SmscSettings settings) throws AdminException{
        settings.addSmsc(smsc);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.addSmsc(smsc.getName());
      }
    });
  }

  /**
   * Обновляет существующий СМСЦ
   * @param smsc СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void updateSmsc(final Smsc smsc) throws AdminException{
    updateSettings(new SettingsWriter<SmscSettings>() {
      public void changeSettings(SmscSettings settings) throws AdminException{
        settings.updateSmsc(smsc);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.updateSmsc(smsc.getName());
      }
    });
  }

  /**
   * Возвращает список СМСЦ
   * @return список СМСЦ
   */
  public List<Smsc> getSmscs() {
    return readSettings(new SettingsReader<SmscSettings,List<Smsc>>(){
      public List<Smsc> executeRead(SmscSettings settings)  {
        List<Smsc> result = new LinkedList<Smsc>();
        for(Smsc s : settings.getSmscs()) {
          result.add(s.cloneSmsc());
        }
        return result;
      }
    });            
  }

  /**
   * ВОзвращает СМСЦ имени
   * @param name имя СМСЦ
   * @return СМСЦ
   */
  public Smsc getSmsc(final String name){
    return readSettings(new SettingsReader<SmscSettings,Smsc>(){
      public Smsc executeRead(SmscSettings settings)  {
        Smsc smsc = settings.getSmsc(name);
        return smsc == null ? null : smsc.cloneSmsc();
      }
    });
  }

  /**
   * Удаляет СМСЦ
   * @param smscName имя СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void removeSmsc(final String smscName) throws AdminException{
    updateSettings(new SettingsWriter<SmscSettings>() {
      public void changeSettings(SmscSettings settings) throws AdminException{
        settings.removeSmsc(smscName);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.removeSmsc(smscName);
      }
    });
  }

  /**
   * Устанавливает СМСЦ по умолчанию
   * @param smsc имя СМСЦ
   * @throws AdminException ошибка сохранения
   */
  public void setDefaultSmsc(final String smsc) throws AdminException {
    updateSettings(new SettingsWriter<SmscSettings>() {
      public void changeSettings(SmscSettings settings) throws AdminException{
        settings.setDefaultSmsc(smsc);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.setDefaultSmsc(smsc);
      }
    });
  }

  /**
   * Возвращает СМСЦ по умолчанию
   * @return СМСЦ по умолчанию
   */
  public String getDefaultSmsc() {
    return readSettings(new SettingsReader<SmscSettings,String>(){
      public String executeRead(SmscSettings settings)  {
        return settings.getDefaultSmsc();
      }
    });
  }

}
