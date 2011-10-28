package ru.novosoft.smsc.web.controllers.sms_view;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemon;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreManager;
import ru.novosoft.smsc.admin.util.DBExportSettings;
import ru.novosoft.smsc.admin.util.ProgressObserver;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class SmsExportController extends SmscController{


  private static final Logger logger = Logger.getLogger(SmsExportController.class);

  private final DBExportSettings defOperExportSettings;
  private final DBExportSettings defArchExportSettings;

  private DBExportSettings settings = new DBExportSettings();

  private Exporter exporter;

  private Date date = new Date();

  private StorageType storageType = StorageType.OPERATIVE;
  private Type type = Type.DEFAULT;

  private State state = State.FILTER;

  private String filterError;

  public SmsExportController() {
    this.defOperExportSettings = WebContext.getInstance().getOperativeStoreManager().getDefExportSettings();
    this.defArchExportSettings = WebContext.getInstance().getArchiveDaemonManager().getDefExportSettings();
  }

  public String apply() {
    if(date == null && storageType == StorageType.ARCHIVE) {
      filterError = getLocalizedString("smsc.sms.export.date.requried");
      return null;
    }else {
      filterError = null;
    }
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.HOUR_OF_DAY, 0);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);

    settings.setDbType(DBExportSettings.DbType.ORACLE);
    exporter = new Exporter(WebContext.getInstance().getOperativeStoreManager(), WebContext.getInstance().getArchiveDaemon(), storageType, c.getTime(), type == Type.DEFAULT ? defOperExportSettings : settings);
    new Thread(exporter).start();
    state = State.PROGRESS;

    return null;
  }

  public String getFilterError() {
    return filterError;
  }

  public String clear() {
    date = new Date();
    settings = new DBExportSettings();
    exporter = null;
    return null;
  }

  public Date getDate() {
    return date;
  }

  public void setDate(Date date) {
    this.date = date;
  }

  public DBExportSettings getSettings() {
    return settings;
  }

  public String more() {
    exporter = null;
    state = State.FILTER;
    return null;
  }

  public StorageType getStorageType() {
    return storageType;
  }

  public void setStorageType(StorageType storageType) {
    this.storageType = storageType;
  }

  public DBExportSettings getDefOperExportSettings() {
    return defOperExportSettings;
  }

  public DBExportSettings getDefArchExportSettings() {
    return defArchExportSettings;
  }

  public int getCurrent() {
    return exporter == null ? 0 : exporter.current;
  }

  public int getTotal() {
    return exporter == null ? Integer.MAX_VALUE : exporter.total;
  }

  public boolean isShowFilter() {
    return state == State.FILTER;
  }

  public int getPercents() {
    if(exporter == null) {
      return 0;
    }
    int total = exporter.total;
    return  total == 0 ? 0 : exporter.current*100/total;
  }

  public boolean isShowProgress() {
    return state == State.PROGRESS;
  }

  public boolean isShowResults() {
    return state == State.RESULTS;
  }

  public String getError() {
    if(exporter == null || exporter.error == null) {
      return null;
    }
    if(exporter.error instanceof AdminException) {
      return ((AdminException) exporter.error).getMessage(getLocale());
    }else {
      return getLocalizedString("smsc.export.internal.error");
    }
  }


  public List<SelectItem> getStorageTypes() {
    StorageType[] types = StorageType.values();
    List<SelectItem> res = new ArrayList<SelectItem>(types.length);
    for(StorageType t : types) {
      if(t == StorageType.ARCHIVE) {
        if(WebContext.getInstance().getArchiveDaemonManager() == null) {
          continue;
        }
      }
      res.add(new SelectItem(t, getLocalizedString("smsc.sms.export.type."+t)));
    }
    return res;
  }

  public Type getType() {
    return type;
  }

  public void setType(Type type) {
    this.type = type;
  }

  public List<SelectItem> getTypes() {
    Type[] types = Type.values();
    List<SelectItem> res = new ArrayList<SelectItem>(types.length);
    for(Type t : types) {
      res.add(new SelectItem(t, getLocalizedString("smsc.export.type." + t)));
    }
    return res;
  }


  public static enum StorageType{
    OPERATIVE, ARCHIVE
  }

  public static enum Type{
    DEFAULT, CUSTOM
  }

  private static enum State{
    FILTER, PROGRESS, RESULTS
  }

  public class Exporter implements Runnable{

    private Exception error;

    private final ArchiveDaemon daemon;
    private final OperativeStoreManager operativeStoreManager;
    private final StorageType storageType;
    private final Date date;
    private final DBExportSettings settings;

    private int total = Integer.MAX_VALUE;
    private int current;

    public Exporter(OperativeStoreManager operativeStoreManager, ArchiveDaemon daemon, StorageType storageType, Date date, DBExportSettings settings) {
      this.operativeStoreManager = operativeStoreManager;
      this.daemon = daemon;
      this.date = date;
      this.settings = settings;
      this.storageType = storageType;
    }

    public void run() {
      try{
        ProgressObserver progressObserver = new ProgressObserverImpl();
        switch (storageType) {
          case OPERATIVE:
            operativeStoreManager.export(settings,progressObserver);
            break;
          case ARCHIVE:
            if(daemon != null) {
              daemon.export(date, settings, progressObserver);
            }
        }
      }catch (Exception e){
        error = e;
        logger.error(e,e);
      }finally {
        state = State.RESULTS;
      }
    }

    private class ProgressObserverImpl implements ProgressObserver {
      public void update(long _current, long _total) {
        current = (int)_current;
        total = (int)_total;
      }
    }
  }


}
