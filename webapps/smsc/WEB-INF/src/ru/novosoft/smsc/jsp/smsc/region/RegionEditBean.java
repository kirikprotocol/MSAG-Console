package ru.novosoft.smsc.jsp.smsc.region;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.region.RegionsManager;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionEditBean extends SmscBean {
  public static final int RESULT_SAVE = SmscBean.PRIVATE_RESULT + 1;
  public static final int RESULT_CANCEL = SmscBean.PRIVATE_RESULT + 2;

  private String mbApply;
  private String mbCancel;

  protected String[] checkedSources = null;
  protected Set checkedSourcesSet = null;

  private String name = null;
  private String id = "";
  private String sendSpeed = null;
  private String email;
  private String timezone;
  private RegionsManager rm;
  protected RouteSubjectManager routeSubjectManager = null;


  protected int init(List errors) {
    int result = super.init(errors);
    if (result != PageBean.RESULT_OK)
      return result;

    rm = appContext.getRegionsManager();
    routeSubjectManager = appContext.getRouteSubjectManager();

    return PageBean.RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != PageBean.RESULT_OK)
      return result;

    if (name == null && request.getParameter("region") != null) {
      try {
        id = request.getParameter("region");
        final Region region = rm.getRegionById(Integer.parseInt(request.getParameter("region")));
        name = region.getName();
        sendSpeed = String.valueOf(region.getBandWidth());
        email = region.getEmail();
        timezone = region.getTimezone().getID();
        checkedSources = new String[region.getSubjects().size()];
        int i = 0;
        for (Iterator iter = region.getSubjects().iterator(); iter.hasNext(); i++) {
          checkedSources[i] = (String)iter.next();
          System.out.println("  " + checkedSources[i]);
        }

      } catch (AdminException e) {
        return error("Can't load region", e);
      }
    } else {
      if (checkedSources == null)
        checkedSources = new String[0];
    }

    checkedSources = Functions.trimStrings(checkedSources);
    checkedSourcesSet = new HashSet(Arrays.asList(checkedSources));

    if (mbApply != null) {
      mbApply = null;
      return save();
    } else if (mbCancel != null) {
      mbCancel = null;
      return cancel();
    }

    return PageBean.RESULT_OK;
  }

  private int save() {

    if (name == null || name.length() == 0)
      return error("Name is empty");
    if (sendSpeed == null || sendSpeed.length() == 0)
      return error("Send speed is empty");
    try {
      Region region;
      if (id != null && id.length() > 0)
        region = rm.getRegionById(Integer.parseInt(id));
      else
        region = new Region(name);

      try {
        region.setName(name);
        region.setBandWidth(Integer.parseInt(sendSpeed));
        region.setEmail(email);
        region.setTimezone(TimeZone.getTimeZone(timezone));
      } catch (Throwable e) {
        return error("Invalid send speed: " + sendSpeed);
      }

      region.clearSubjects();
      for (Iterator iter = checkedSourcesSet.iterator(); iter.hasNext();)
        region.addSubject((String)iter.next());


      rm.addRegion(region);
    } catch (AdminException e) {
      return error("Can't save region", e);
    }

    return RESULT_SAVE;
  }

  public boolean isSrcChecked(final String srcName){
    return checkedSourcesSet.contains(srcName);
  }

  public Collection getAllSubjects() {
    return routeSubjectManager.getSubjects().getNames();
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

  public String getSendSpeed() {
    return sendSpeed;
  }

  public void setSendSpeed(String sendSpeed) {
    this.sendSpeed = sendSpeed;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) {
    this.email = email;
  }

  public String[] getCheckedSources() {
    return checkedSources;
  }

  public void setCheckedSources(final String[] checkedSources) {
    this.checkedSources = checkedSources;
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getTimezone() {
    return timezone;
  }

  public void setTimezone(String timezone) {
    this.timezone = timezone;
  }

  public Collection getTimezones() {
    List c = Arrays.asList(TimeZone.getAvailableIDs());
    Collections.sort(c);
    return c;
  }
}
