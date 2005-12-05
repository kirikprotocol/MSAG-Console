package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

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
  private int defaultInformId = -1;
  private int defaultNotifyId = -1;

  private String editTemplate = null;

  private String[]   informChecked    = new String[0];
  private Collection informCheckedSet = null;
  private String[]   notifyChecked    = new String[0];
  private Collection notifyCheckedSet = null;

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

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)  return result;

    informCheckedSet = new HashSet(Arrays.asList(informChecked));
    notifyCheckedSet = new HashSet(Arrays.asList(notifyChecked));

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int superResult = super.process(request);
    int result = setDefaultTemplateIds(getConfig());
    if (superResult != RESULT_OK) return superResult;
    if (result != RESULT_OK) return result;

    if      (mbInformAdd != null)    return RESULT_INFORM_ADD;
    else if (mbInformEdit != null)   return RESULT_INFORM_EDIT;
    else if (mbNotifyAdd  != null)   return RESULT_NOTIFY_ADD;
    else if (mbNotifyEdit != null)   return RESULT_NOTIFY_EDIT;
    else if (mbInformDelete != null) result = informDelete();
    else if (mbNotifyDelete != null) result = notifyDelete();

    int loadResult = loadupFromConfig(getConfig());
    return (result != RESULT_OK) ? result:loadResult;
  }

  private final static String informTemplateIdParam = INFORM_TEMPLATES_SECTION_NAME+".default";
  private final static String notifyTemplateIdParam = NOTIFY_TEMPLATES_SECTION_NAME+".default";

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
      return error("mcisme.error.template_load_i", e);
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
      return error("mcisme.error.template_load_n", e);
    }

    return RESULT_OK;
  }

  private int setDefaultTemplateIds(Config config)
  {
    try {
      int informTemplateId = config.getInt(informTemplateIdParam);
      if (defaultInformId < 0) defaultInformId = informTemplateId;
      else if (informTemplateId != defaultInformId) {
        config.setInt(informTemplateIdParam, defaultInformId);
        getMCISmeContext().setChangedTemplates(true);
      }
      int notifyTemplateId = config.getInt(notifyTemplateIdParam);
      if (defaultNotifyId < 0) defaultNotifyId = notifyTemplateId;
      else if (notifyTemplateId != defaultNotifyId) {
        config.setInt(notifyTemplateIdParam, defaultNotifyId);
        getMCISmeContext().setChangedTemplates(true);
      }
    } catch (Exception e) {
      return error("mcisme.error.template_default", e);
    }
    return RESULT_OK;
  }

  private int informDelete()
  {
    boolean needWarning = false; String defaultTemplateName = "";
    try
    {
      final String informTemplatePrefix = INFORM_TEMPLATES_SECTION_NAME+'.';
      for (int i = 0; i < informChecked.length; i++)
      {
        String section = informTemplatePrefix+StringEncoderDecoder.encodeDot(informChecked[i]);
        if (getConfig().getInt(section+".id") == defaultInformId) {
          needWarning = true; defaultTemplateName = informChecked[i];
        } else {
          getConfig().removeSection(section);
          getMCISmeContext().setChangedTemplates(true);
        }
        informCheckedSet.remove(informChecked[i]);
      }
    } catch (Exception e) {
      return error("mcisme.error.template_del_i", e);
    }
    return (needWarning) ? warning("mcisme.warn.template_del_i", defaultTemplateName) : RESULT_OK;
  }
  private int notifyDelete()
  {
    boolean needWarning = false; String defaultTemplateName = "";
    try
    {
      final String notifyTemplatePrefix = NOTIFY_TEMPLATES_SECTION_NAME+'.';
      for (int i = 0; i < notifyChecked.length; i++) {
        String section = notifyTemplatePrefix+StringEncoderDecoder.encodeDot(notifyChecked[i]);
        if (getConfig().getInt(section+".id") == defaultNotifyId) {
          needWarning = true; defaultTemplateName = notifyChecked[i];
        } else {
          getConfig().removeSection(section);
          getMCISmeContext().setChangedTemplates(true);
        }
        notifyCheckedSet.remove(notifyChecked[i]);
      }
    } catch (Exception e) {
      return error("mcisme.error.template_del_n", e);
    }
    return (needWarning) ? warning("mcisme.warn.template_del_n", defaultTemplateName) : RESULT_OK;
  }

  public boolean isInformTemplateChecked(String templateName) {
    return informCheckedSet.contains(templateName);
  }
  public boolean isNotifyTemplateChecked(String templateName) {
    return notifyCheckedSet.contains(templateName);
  }

  public String[] getInformChecked() {
    return informChecked;
  }
  public void setInformChecked(String[] informChecked) {
    this.informChecked = informChecked;
  }
  public String[] getNotifyChecked() {
    return notifyChecked;
  }
  public void setNotifyChecked(String[] notifyChecked) {
    this.notifyChecked = notifyChecked;
  }
  public Collection getInformCheckedSet() {
    return informCheckedSet;
  }
  public Collection getNotifyCheckedSet() {
    return notifyCheckedSet;
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
