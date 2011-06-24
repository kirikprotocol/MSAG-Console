package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.config.InformerTimezone;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class RegionsController extends InformerController {

  final static String REGION_ID_PARAMETER = "REGION_ID";

  public static final String REGION_IDS_PARAM = "region_ids";

  protected final Collection<Smsc> ss;

  private final List<SelectItem> timeZones = new LinkedList<SelectItem>();

  protected RegionsController() {
    ss = getConfig().getSmscs();

    Locale l = getLocale();
    for (InformerTimezone t : WebContext.getInstance().getWebTimezones().getTimezones())
      timeZones.add(new SelectItem(t.getTimezone(), t.getAlias(l)));

    Collections.sort(timeZones, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
  }

  public List<SelectItem> getSmscs() {
    List<SelectItem> smscs = new ArrayList<SelectItem>(ss.size());
    for (Smsc s : ss) {
      smscs.add(new SelectItem(s.getName(), s.getName()));
    }
    Collections.sort(smscs, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return smscs;
  }

  public List<SelectItem> getTimeZones() {
    return timeZones;
  }

}
