package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.config.InformerTimezone;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
public class RegionEditGroupController extends RegionsController {

  private List<String> ids;

  private boolean editTimeZone;
  private TimeZone timeZone;

  private boolean editMaxSmsPerSecond;
  private int maxSmsPerSecond;

  private boolean editSmsc;
  private String smsc;

  private Configuration config;

  private final List<SelectItem> timeZones = new LinkedList<SelectItem>();

  public RegionEditGroupController() {
    Object o = getRequest().get(REGION_IDS_PARAM);
    if(o != null) {
      ids =  (List<String>)o;
    }
    config = getConfig();

    Locale l = getLocale();
    for (InformerTimezone t : WebContext.getInstance().getWebTimezones().getTimezones())
      timeZones.add(new SelectItem(t.getTimezone(), t.getAlias(l)));
  }


  public String save() {
    try{
      for(String i : ids) {
        Region d = config.getRegion(Integer.parseInt(i));
        if(d == null) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "region.group.not.found", i);
        } else {
          if(editMaxSmsPerSecond) {
            d.setMaxSmsPerSecond(maxSmsPerSecond);
          }
          if(editSmsc) {
            d.setSmsc(smsc);
          }
          if(editTimeZone) {
            d.setTimeZone(timeZone);
          }
          config.updateRegion(d, getUserName());
        }
      }
      return "REGION";
    }catch (AdminException e){
      addError(e);
    }
    return null;
  }

  public List<String> getIds() {
    return ids;
  }

  public void setIds(List<String> ids) {
    this.ids = ids;
  }

  public boolean isEditTimeZone() {
    return editTimeZone;
  }

  public void setEditTimeZone(boolean editTimeZone) {
    this.editTimeZone = editTimeZone;
  }

  public TimeZone getTimeZone() {
    return timeZone;
  }

  public void setTimeZone(TimeZone timeZone) {
    this.timeZone = timeZone;
  }

  public boolean isEditMaxSmsPerSecond() {
    return editMaxSmsPerSecond;
  }

  public void setEditMaxSmsPerSecond(boolean editMaxSmsPerSecond) {
    this.editMaxSmsPerSecond = editMaxSmsPerSecond;
  }

  public int getMaxSmsPerSecond() {
    return maxSmsPerSecond;
  }

  public void setMaxSmsPerSecond(int maxSmsPerSecond) {
    this.maxSmsPerSecond = maxSmsPerSecond;
  }

  public boolean isEditSmsc() {
    return editSmsc;
  }

  public void setEditSmsc(boolean editSmsc) {
    this.editSmsc = editSmsc;
  }

  public String getSmsc() {
    return smsc;
  }

  public void setSmsc(String smsc) {
    this.smsc = smsc;
  }
}
