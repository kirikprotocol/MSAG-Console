package ru.novosoft.smsc.web.controllers.stat;


import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.stat.ExportResults;
import ru.novosoft.smsc.admin.stat.SmscStatFilter;
import ru.novosoft.smsc.admin.stat.SmscStatLoadListener;
import ru.novosoft.smsc.admin.stat.SmscStatProvider;
import ru.novosoft.smsc.admin.util.DBExportSettings;
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
public class SmscStatExportController extends SmscController{

  private static final Logger logger = Logger.getLogger(SmscStatExportController.class);

  private DBExportSettings defExportSettings;

  private DBExportSettings settings = new DBExportSettings();

  private Exporter exporter;

  private Date date = new Date();

  private Type type = Type.DEFAULT;

  private State state = State.FILTER;

  public SmscStatExportController() {
    this.defExportSettings = WebContext.getInstance().getSmscStatProvider().getDefExportSettings();
  }

  public String apply() {
    if(date != null) {
      SmscStatFilter filter = new SmscStatFilter();

      Calendar c = Calendar.getInstance();
      c.setTime(date);
      c.set(Calendar.HOUR_OF_DAY, 0);
      c.set(Calendar.MINUTE, 0);
      c.set(Calendar.SECOND, 0);
      c.set(Calendar.MILLISECOND, 0);
      filter.setFrom(c.getTime());
      c.add(Calendar.DAY_OF_MONTH, 1);
      filter.setTill(c.getTime());

      exporter = new Exporter(WebContext.getInstance().getSmscStatProvider(), filter, type == Type.DEFAULT ? defExportSettings : settings);
      new Thread(exporter).start();
      state = State.PROGRESS;
    }
    return null;
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

  public Type getType() {
    return type;
  }

  public void setType(Type type) {
    this.type = type;
  }

  public DBExportSettings getDefExportSettings() {
    return defExportSettings;
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

  public ExportResults getResults() {
    return exporter == null ? null : exporter.results;
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


  public List<SelectItem> getTypes() {
    Type[] types = Type.values();
    List<SelectItem> res = new ArrayList<SelectItem>(types.length);
    for(Type t : types) {
      res.add(new SelectItem(t, getLocalizedString("smsc.export.type."+t)));
    }
    return res;
  }


  public List<SelectItem> getDbTypes() {
    DBExportSettings.DbType[] types = DBExportSettings.DbType.values();
    List<SelectItem> res = new ArrayList<SelectItem>(types.length);
    for(DBExportSettings.DbType t : types) {
      res.add(new SelectItem(t, t.toString()));
    }
    return res;
  }


  public static enum Type{
    DEFAULT, CUSTOM
  }

  private static enum State{
    FILTER, PROGRESS, RESULTS
  }

  public class Exporter implements Runnable{

    private Exception error;

    private final SmscStatProvider provider;
    private final SmscStatFilter filter;
    private final DBExportSettings settings;

    private int total = Integer.MAX_VALUE;
    private int current;

    private ExportResults results;

    public Exporter(SmscStatProvider provider, SmscStatFilter filter, DBExportSettings settings) {
      this.provider = provider;
      this.filter = filter;
      this.settings = settings;
    }

    public void run() {
      try{
        results = provider.exportStatistics(filter, settings, new SmscStatLoadListener() {
          public void incrementProgress() {
            current++;
          }
          public void setTotal(int numb) {
            total = numb;
          }
        });
      }catch (Exception e){
        error = e;
        logger.error(e,e);
      }finally {
        state = State.RESULTS;
      }
    }
  }

}
