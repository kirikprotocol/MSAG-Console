package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Set;
import java.util.Iterator;
import java.net.URLEncoder;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 16.07.2004
 * Time: 14:45:52
 * To change this template use File | Settings | File Templates.
 */
public class Template extends MCISmeBean
{
  private int     templateId   = 0;
  private String  templateName = "";
  private String  message      = "";
  private String  multiRow     = "";
  private String  singleRow    = "";
  private boolean group        = false;

  private boolean initialized     = false;
  private boolean informTemplate  = false;
  private boolean createTemplate  = false;
  private String  oldTemplateName = null;

  private String mbDone   = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (!initialized && !createTemplate)
    {
      if (templateName == null || templateName.length() == 0)
        return error("Template name not specified");
      result = loadFromConfig(getConfig(), templateName);
      if (result != RESULT_OK) return result;
      oldTemplateName = templateName;
    }
    if (oldTemplateName == null) oldTemplateName = "";

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (mbCancel != null)    return RESULT_DONE;
    else if (mbDone != null) return done();

    return result;
  }

  private int done()
  {
    if (templateName == null || templateName.length() == 0)
      return error("Template name not specified");

    Config config = getConfig();
    final String defaultIdParam = getTemplateSectionPrefix() + ".default";
    int defaultTemplateId = getIdFromConfig(config, defaultIdParam);

    if (createTemplate) // create new template
    {
      if (containsInConfig(config, templateName))
        return error("Template '"+templateName+"' already exists");
      if (containsInConfig(config, templateId, null))
        return error("Template with id="+templateId+" already exists");
      if (defaultTemplateId < 0)
        config.setInt(defaultIdParam, templateId);
    }
    else // edit old template
    {
      if ( oldTemplateName != null && oldTemplateName.length() > 0 &&
          !oldTemplateName.equals(templateName) &&
          containsInConfig(config, templateName))
        return error("Template '"+templateName+"' already exists");
      if (containsInConfig(config, templateId, oldTemplateName))
        return error("Template with id="+templateId+" already exists");

      if ( oldTemplateName != null && oldTemplateName.length() > 0 )
      {
        final String oldTemplateIdParam = getTemplateSectionPrefix()+'.'+
              StringEncoderDecoder.encodeDot(oldTemplateName)+".id";
        int oldTemplateId = getIdFromConfig(config, oldTemplateIdParam);
        if (defaultTemplateId < 0 || defaultTemplateId == oldTemplateId)
          config.setInt(defaultIdParam, templateId);

        removeFromConfig(config, oldTemplateName);
      }
    }

    storeToConfig(config, templateName);
    getMCISmeContext().setChangedTemplates(true);
    return RESULT_DONE;
  }

  private String getTemplateSectionPrefix() {
    return (informTemplate) ? INFORM_TEMPLATES_SECTION_NAME : NOTIFY_TEMPLATES_SECTION_NAME;
  }
  private boolean containsInConfig(Config config, String name) {
    return config.containsSection(getTemplateSectionPrefix()+'.'+StringEncoderDecoder.encodeDot(name));
  }
  private boolean containsInConfig(Config config, int id, String exceptName)
  {
    Set set = config.getSectionChildSectionNames(getTemplateSectionPrefix());
    for (Iterator i = set.iterator(); i.hasNext();) {
      String section = (String)i.next();
      try {
        if (exceptName != null && exceptName.length() > 0 && section.endsWith(exceptName)) continue;
        if (config.getInt(section+".id") == id) return true;
      } catch (Exception e) { e.printStackTrace(); }
    }
    return false;
  }
  private void removeFromConfig(Config config, String name) {
    config.removeSection(getTemplateSectionPrefix()+'.'+ StringEncoderDecoder.encodeDot(name));
  }
  private int getIdFromConfig(Config config, String param) {
    try { return config.getInt(param); }
    catch (Exception e) { e.printStackTrace(); }
    return -1;
  }
  private void storeToConfig(Config config, String name)
  {
    final String prefix = getTemplateSectionPrefix()+'.'+StringEncoderDecoder.encodeDot(name);
    config.setInt(prefix + ".id", templateId);
    config.setString(prefix + ".message", message);
    if (informTemplate) {
      config.setBool(prefix + ".group", group);
      config.setString(prefix + ".singleRow", singleRow);
      if (group) config.setString(prefix + ".multiRow", multiRow);
    }
  }
  private int loadFromConfig(Config config, String name)
  {
    final String prefix = getTemplateSectionPrefix()+'.'+StringEncoderDecoder.encodeDot(name);
    try
    {
      templateId = config.getInt(prefix + ".id");
      message = config.getString(prefix + ".message");
      if (informTemplate) {
        group = config.getBool(prefix + ".group");
        singleRow = config.getString(prefix + ".singleRow");
        multiRow = (group) ? config.getString(prefix + ".multiRow"):"";
      } else {
        group = false; singleRow = ""; multiRow = "";
      }
    }
    catch (Exception e) {
      return error("Load from config failed", e);
    }
    return RESULT_OK;
  }

  public boolean isInitialized() {
    return initialized;
  }
  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }
  public String getOldTemplateName() {
    return oldTemplateName;
  }
  public void setOldTemplateName(String oldTemplateName) {
    this.oldTemplateName = oldTemplateName;
  }

  public int getTemplateId() {
    return templateId;
  }
  public void setTemplateId(int templateId) {
    this.templateId = templateId;
  }

  public String getTemplateName() {
    return templateName;
  }
  public void setTemplateName(String templateName) {
    this.templateName = templateName;
  }

  public String getMessage() {
    return message;
  }
  public void setMessage(String message) {
    this.message = message;
  }

  public String getMultiRow() {
    return multiRow;
  }
  public void setMultiRow(String multiRow) {
    this.multiRow = multiRow;
  }

  public String getSingleRow() {
    return singleRow;
  }
  public void setSingleRow(String singleRow) {
    this.singleRow = singleRow;
  }

  public boolean isGroup() {
    return group;
  }
  public void setGroup(boolean group) {
    this.group = group;
  }

  public boolean isInformTemplate() {
    return informTemplate;
  }
  public void setInformTemplate(boolean informTemplate) {
    this.informTemplate = informTemplate;
  }
  public boolean isCreateTemplate() {
    return createTemplate;
  }
  public void setCreateTemplate(boolean createTemplate) {
    this.createTemplate = createTemplate;
  }

  public String getMbDone() {
    return mbDone;
  }
  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }
  public String getMbCancel() {
    return mbCancel;
  }
  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }
}
