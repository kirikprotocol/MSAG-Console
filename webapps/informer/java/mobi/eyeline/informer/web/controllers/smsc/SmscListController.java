package mobi.eyeline.informer.web.controllers.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;

import javax.faces.component.UIComponent;
import javax.faces.event.ValueChangeEvent;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SmscListController extends SmscController{

  private List<RadioSmsc> smscs;

  private String defaultSmsc;


  public SmscListController() {
    defaultSmsc = getConfig().getDefaultSmsc();
    smscs = new LinkedList<RadioSmsc>();
    for(Smsc s : getConfig().getSmscs()) {
      smscs.add(new RadioSmsc(s, defaultSmsc.equals(s.getName())));
    }

    Collections.sort(smscs, new Comparator<RadioSmsc>() {
      public int compare(RadioSmsc o1, RadioSmsc o2) {
        return o1.getName().compareTo(o2.getName());
      }
    });
  }

  public String getDefaultSmsc() {
    return defaultSmsc;
  }

  public List<RadioSmsc> getSmscs() {
    return smscs;
  }

  public void change(ValueChangeEvent e) {
    Boolean newV = (Boolean)e.getNewValue();
    if(newV) {
      UIComponent c = (UIComponent)e.getSource();
      try {
        String smsc = (String)c.getAttributes().get(SMSC_PARAMETER);
        getConfig().setDefaultSmsc(smsc, getUserName());
        defaultSmsc = smsc;
      } catch (AdminException e1) {
        addError(e1);
      }
    }
  }

  public String remove() {
    String smsc = getRequestParameter(SMSC_PARAMETER);
    if(smsc != null) {
      try{
        getConfig().removeSmsc(smsc, getUserName());
      }catch (AdminException e){
        addError(e);
        return null;
      }
      Iterator<RadioSmsc> i = smscs.iterator();
      while(i.hasNext()) {
        if(i.next().getName().equals(smsc)) {
          i.remove();
          break;
        }
      }
    }
    return null;
  }

  public static class RadioSmsc {

    private Smsc smsc;

    private boolean def;

    public RadioSmsc(Smsc smsc, boolean def) {
      this.smsc = smsc;
      this.def = def;
    }

    public boolean isDef() {
      return def;
    }

    public void setDef(boolean def) {
      this.def = def;
    }

    public String getName() {
      return smsc.getName();
    }

    public String getHost() {
      return smsc.getHost();
    }

    public int getPort() {
      return smsc.getPort();
    }
  }
}
