package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.*;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public abstract class SmsViewController extends SmscController{

  protected static final Logger logger = Logger.getLogger(SmsViewController.class);

  private LoadListener loadListener;

  protected WebContext wcontext = WebContext.getInstance();

  private boolean loaded;

  protected void loadingIsNeeded() {
    loaded = false;
    loadListener = null;
  }

  public List<SelectItem> getMaxRows() {
    List<SelectItem> res = new ArrayList<SelectItem>();
    Collections.addAll(res,
        new SelectItem(10),
        new SelectItem(100),
        new SelectItem(200),
        new SelectItem(300),
        new SelectItem(400),
        new SelectItem(500),
        new SelectItem(1000),
        new SelectItem(2000),
        new SelectItem(5000)
    );
    return res;
  }


  public boolean isLoaded() {
    return loaded;
  }



  protected abstract void _load(Locale locale, LoadListener loadListener) throws AdminException;

  protected LoadListener load(final Locale locale) {
    LoadListener listener = null;
    if(!loaded) {
      if(loadListener == null) {
        loadListener = new LoadListener();
        new Thread() {
          public void run() {
            try{
              _load(locale, loadListener);
              loaded = true;
            }catch (AdminException e){
              logger.error(e,e);
              loadListener.setLoadError(new ModelException(e.getMessage(locale)));
            }catch (Exception e){
              logger.error(e, e);
              loadListener.setLoadError(new ModelException(e.getLocalizedMessage() != null ? e.getLocalizedMessage() : e.getMessage() != null
                  ? e.getMessage() : "Internal error"));
            }
          }
        }.start();
      }
      listener = loadListener;
    }
    return listener;
  }

  private static final String smppStatusPrefix = "smsc.errcode.";

  public List<SelectItem> getLastResults() {
    Locale locale = getLocale();
    ResourceBundle bundle = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", locale);
    Enumeration<String> enumeration = bundle.getKeys();
    Map<Integer,String> preres = new TreeMap<Integer, String>();
    while (enumeration.hasMoreElements()) {
      String key = enumeration.nextElement();
      if(key.length() > smppStatusPrefix.length() && key.startsWith(smppStatusPrefix)) {
        try{
          Integer code = Integer.parseInt(key.substring(smppStatusPrefix.length()));
          preres.put(code, "(" + code + ") " + bundle.getString(key));
        }catch (NumberFormatException ignored){}
      }
    }

    List<SelectItem> res = new ArrayList<SelectItem>(preres.size());
    for(Map.Entry<Integer, String> e : preres.entrySet()) {
      res.add(new SelectItem(e.getKey().toString(), e.getValue()));
    }

    return res;
  }

  protected abstract List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder);

  protected abstract int getRowsCount();

  public DataTableModel getMessages() {

    final Locale locale = getLocale();
    return new PreloadableModel() {

      public LoadListener prepareRows(int i, int i1, DataTableSortOrder dataTableSortOrder) throws ModelException {
        return load(locale);
      }
      public List getRows(int startPos, int count, DataTableSortOrder dataTableSortOrder) throws ModelException {
        return SmsViewController.this.getRows(startPos, count, dataTableSortOrder);
      }
      public int getRowsCount() throws ModelException {
        return SmsViewController.this.getRowsCount();
      }
    };

  }

}
