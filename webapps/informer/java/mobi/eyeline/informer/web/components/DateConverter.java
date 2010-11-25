package mobi.eyeline.informer.web.components;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Calendar;
import java.util.StringTokenizer;

/**
 * @author Artem Snopkov
 */
public class DateConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {

    System.out.println("GET AS OBJECT: " + s);     //todo

    StringTokenizer st = new StringTokenizer(s, "\n");
    String t = st.nextToken();
    Calendar c = Calendar.getInstance();

    String hour = "";

    if(t.startsWith("year")) {
      String year, month, day;
      year = t.substring(5);
      if (year.trim().length() == 0 || year.trim().equals("null")) {
        return null;
      }


      month = st.nextToken().substring(6);
      if (month.trim().length() == 0 || month.equals("-1") || month.trim().equals("null")){
        return null;
      }

      day = st.nextToken().substring(4);
      if (day.trim().length() == 0 || day.trim().equals("null")){
        return null;
      }

      c.set(Calendar.YEAR, Integer.parseInt(year));
      c.set(Calendar.MONTH, Integer.parseInt(month) - 1);
      c.set(Calendar.DATE, Integer.parseInt(day));
      if (st.hasMoreTokens())
        hour = st.nextToken().substring(6);
      if (hour.trim().length() == 0)
        hour="00";
    }else {
      hour = t.substring(6);
      if (hour.trim().length() == 0 || hour.trim().equals("null"))
        return null;   
    }

    String minutes = "";
    if (st.hasMoreTokens())
      minutes = st.nextToken().substring(8);
    if (minutes.trim().length() == 0)
      minutes="00";

    String seconds = "";
    if (st.hasMoreTokens())
      seconds = st.nextToken().substring(8);
    if (seconds.trim().length() == 0)
      seconds="00";
    c.set(Calendar.HOUR_OF_DAY, Integer.parseInt(hour));
    c.set(Calendar.MINUTE, Integer.parseInt(minutes));
    c.set(Calendar.SECOND, Integer.parseInt(seconds));

    return c.getTime();
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    System.out.println("GET AS STRING: " + o);     //todo
    return null;
  }
}