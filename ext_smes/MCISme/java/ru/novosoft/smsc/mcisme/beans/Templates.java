package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 15.07.2004
 * Time: 15:33:30
 * To change this template use File | Settings | File Templates.
 */
public class Templates extends MCISmeBean
{
  private int defaultInformId = 0;
  private int defaultNotifyId = 0;

  private String editTemplate = null;

  private String mbInformAdd = null;
  private String mbInformEdit = null;
  private String mbInformDelete = null;
  private String mbNotifyAdd = null;
  private String mbNotifyEdit = null;
  private String mbNotifyDelete = null;

  public class Identity {
    public int    id = 0;
    public String name = "";

    public Identity() {}
    public Identity(int id, String name) {
      this.id = id; this.name = name;
    }
  }
  private class IdentityComparator implements Comparator
  {
    public int compare(Object o1, Object o2) {
      if (o1 != null && o1 instanceof Identity && o2 != null && o2 instanceof Identity) {
        Identity i1 = (Identity)o1; Identity i2 = (Identity)o2;
        return (i1.id-i2.id);
      }
      return 0;
    }
  }

  private SortedList informTemplates = new SortedList(new IdentityComparator());
  private SortedList notifyTemplates = new SortedList(new IdentityComparator());

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    result = setDefaultTemplateIds(getConfig());
    if (result != RESULT_OK) return result;

    if      (mbInformAdd != null)  return RESULT_INFORM_ADD;
    else if (mbInformEdit != null) return RESULT_INFORM_EDIT;
    else if (mbNotifyAdd  != null) return RESULT_NOTIFY_ADD;
    else if (mbNotifyEdit != null) return RESULT_NOTIFY_EDIT;

    //TODO: templates deleting
    /*else if (mbInformDelete != null)
    else if (mbNotifyDelete != null)*/

    return loadupFromConfig(getConfig());
  }

  private int loadupFromConfig(Config config)
  {
    try {
      defaultInformId = config.getInt(INFORM_TEMPLATES_SECTION_NAME+".default");
      Set set = config.getSectionChildShortSectionNames(INFORM_TEMPLATES_SECTION_NAME);
      for (Iterator i = set.iterator(); i.hasNext();) {
        String section = (String)i.next();
        int id = config.getInt(INFORM_TEMPLATES_SECTION_NAME+'.'+section+".id");
        informTemplates.add(new Identity(id, section));
      }
    } catch (Exception e) {
      return error("Failed to load inform templates", e);      
    }
    
    try {
      defaultNotifyId = config.getInt(NOTIFY_TEMPLATES_SECTION_NAME+".default");
      Set set = config.getSectionChildShortSectionNames(NOTIFY_TEMPLATES_SECTION_NAME);
      for (Iterator i = set.iterator(); i.hasNext();) {
        String section = (String)i.next();
        int id = config.getInt(NOTIFY_TEMPLATES_SECTION_NAME+'.'+section+".id");
        notifyTemplates.add(new Identity(id, section));
      }
    } catch (Exception e) {
      return error("Failed to load notify templates", e);
    }

    return RESULT_OK;
  }

  private int setDefaultTemplateIds(Config config)
  {
    final String informTemplateIdParam = INFORM_TEMPLATES_SECTION_NAME+".default";
    final String notifyTemplateIdParam = NOTIFY_TEMPLATES_SECTION_NAME+".default";
    try {
      if (config.getInt(informTemplateIdParam) != defaultInformId) {
        config.setInt(informTemplateIdParam, defaultInformId);
        getMCISmeContext().setChangedTemplates(true);
      }
      if (config.getInt(notifyTemplateIdParam) != defaultNotifyId) {
        config.setInt(notifyTemplateIdParam, defaultNotifyId);
        getMCISmeContext().setChangedTemplates(true);
      }
    } catch (Exception e) {
      return error("Failed to check default template id", e);
    }
    return RESULT_OK;
  }

  public List getInformTemplates() {
    return informTemplates;
  }
  public List getNotifyTemplates() {
    return notifyTemplates;
  }

  public int getDefaultInformId() {
    return defaultInformId;
  }
  public void setDefaultInformId(int defaultInformId) {
    this.defaultInformId = defaultInformId;
  }
  public int getDefaultNotifyId() {
    return defaultNotifyId;
  }
  public void setDefaultNotifyId(int defaultNotifyId) {
    this.defaultNotifyId = defaultNotifyId;
  }

  public String getEditTemplate() {
    return editTemplate;
  }
  public void setEditTemplate(String editTemplate) {
    this.editTemplate = editTemplate;
  }

  public String getMbInformAdd() {
    return mbInformAdd;
  }
  public void setMbInformAdd(String mbInformAdd) {
    this.mbInformAdd = mbInformAdd;
  }
  public String getMbInformEdit() {
    return mbInformEdit;
  }
  public void setMbInformEdit(String mbInformEdit) {
    this.mbInformEdit = mbInformEdit;
  }
  public String getMbInformDelete() {
    return mbInformDelete;
  }
  public void setMbInformDelete(String mbInformDelete) {
    this.mbInformDelete = mbInformDelete;
  }

  public String getMbNotifyAdd() {
    return mbNotifyAdd;
  }
  public void setMbNotifyAdd(String mbNotifyAdd) {
    this.mbNotifyAdd = mbNotifyAdd;
  }
  public String getMbNotifyEdit() {
    return mbNotifyEdit;
  }
  public void setMbNotifyEdit(String mbNotifyEdit) {
    this.mbNotifyEdit = mbNotifyEdit;
  }
  public String getMbNotifyDelete() {
    return mbNotifyDelete;
  }
  public void setMbNotifyDelete(String mbNotifyDelete) {
    this.mbNotifyDelete = mbNotifyDelete;
  }

}
