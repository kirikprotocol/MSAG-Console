package ru.novosoft.smsc.jsp.smsc.network_profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.network_profiles.Mask;
import ru.novosoft.smsc.admin.network_profiles.NetworkProfile;
import ru.novosoft.smsc.admin.network_profiles.NetworkProfilesManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfileEditBean extends SmscBean {

  public static final int RESULT_SAVE = SmscBean.PRIVATE_RESULT + 1;
  public static final int RESULT_CANCEL = SmscBean.PRIVATE_RESULT + 2;

  private String mbApply;
  private String mbCancel;


  private String name = null;
  private String oldName = null;
  private String abonentStatusMethod = null;
  private String ussdOpenDestRef = null;

  private NetworkProfilesManager nm;

  private final MasksHelper masksHelper = new MasksHelper("network.masks", "mask", 400);

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != PageBean.RESULT_OK)
      return result;

    nm = appContext.getNetworkProfilesManager();

    return PageBean.RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != PageBean.RESULT_OK)
      return result;

    System.out.println("Network: "+request.getParameter("network"));

    if (request.getParameter("network") != null) {
      try {
        oldName = request.getParameter("network");
        final NetworkProfile network = nm.getProfile(oldName);
        name = network.getName();
        abonentStatusMethod = network.getAbonentStatusMethod();
        ussdOpenDestRef = network.getUssdOpenDestRef();
        System.out.println("Fill dynamic table: "+network.getMasks().values());
        masksHelper.fillMasks(network.getMasks().values());

      } catch (AdminException e) {
        return error("Can't load region", e);
      }
    }

    if (mbApply != null) {
      mbApply = null;
      return save();
    } else if (mbCancel != null) {
      mbCancel = null;
      return cancel();
    }

    return PageBean.RESULT_OK;
  }

  public MasksHelper getMasksHelper() {
    return masksHelper;
  }

  public List getAbonentStateMethods() {
    List list = new ArrayList(2);
    list.add("SRI4SM");
    list.add("ATI");
    return list;
  }

  public List getUssdOpenDestRefs() {
    List list = new ArrayList(3);
    list.add("");
    list.add("destAddr");
    list.add("destIMSI");
    return list;
  }

  private int save() {

    try {
      NetworkProfile network;
      if (oldName != null && oldName.length() > 0) {
        network = nm.getProfile(oldName);
        network.setName(name);
      } else {
        network = new NetworkProfile(name);
      }

      network.setAbonentStatusMethod(abonentStatusMethod);
      network.setUssdOpenDestRef(ussdOpenDestRef);

      Map newMasks = new HashMap();
      Iterator i = masksHelper.getMasks().iterator();
      while(i.hasNext()) {
        Mask m = (Mask)i.next();
        newMasks.put(m.getMaskSimple(), m);
      }
      network.setMasks(newMasks);

      if(!name.equals(oldName)) {
        nm.removeProfile(oldName);
      }
      if(oldName == null || oldName.length() == 0) {
        nm.addProfile(network);
      }else {
        nm.editProfile(network);      }

    } catch (AdminException e) {
      return error("Can't save region", e);
    }

    return RESULT_SAVE;
  }
  private int cancel() {
    return RESULT_CANCEL;
  }

  public String getMbApply() {
    return mbApply;
  }


  public void setMbApply(String mbApply) {
    this.mbApply = mbApply;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }


  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getAbonentStatusMethod() {
    return abonentStatusMethod;
  }

  public void setAbonentStatusMethod(String abonentStatusMethod) {
    this.abonentStatusMethod = abonentStatusMethod;
  }

  public String getUssdOpenDestRef() {
    return ussdOpenDestRef;
  }

  public void setUssdOpenDestRef(String ussdOpenDestRef) {
    this.ussdOpenDestRef = ussdOpenDestRef;
  }

  public String getOldName() {
    return oldName;
  }

  public void setOldName(String oldName) {
    this.oldName = oldName;
  }
}
