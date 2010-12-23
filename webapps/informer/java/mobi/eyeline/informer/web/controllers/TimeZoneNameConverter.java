package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.web.LocaleFilter;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.InformerTimezone;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
public class TimeZoneNameConverter implements Converter {


  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if (s == null || (s = s.trim()).length() == 0) {
      return null;
    }

    Locale l = (Locale) FacesContext.getCurrentInstance().getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    InformerTimezone tz = WebContext.getInstance().getWebTimezones().getTimezoneByAlias(s, l);
    return tz == null ? null : tz.getTimezone();
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (!(o instanceof TimeZone)) {
      return null;
    }
    Locale l = (Locale) FacesContext.getCurrentInstance().getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);

    InformerTimezone tz = WebContext.getInstance().getWebTimezones().getTimezoneByID(((TimeZone)o).getID());
    if (tz != null)
      return tz.getAlias(l);

    return ((TimeZone) o).getDisplayName(l == null ? new Locale("en") : l);
  }
}
