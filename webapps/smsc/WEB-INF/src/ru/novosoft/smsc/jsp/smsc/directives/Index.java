package ru.novosoft.smsc.jsp.smsc.directives;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 23.10.2003
 * Time: 14:44:48
 */
public class Index extends PageBean
{
  public static final String[] directives = {"def", "template", "ack", "noack", "hide", "unhide", "flash"};

  private Map aliases = new TreeMap();
  private String mbDone = null;
  private String mbCancel = null;
  private String[] alias = new String[0];
  private String newAlias = "";
  private String newDirective = "";

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    final Config config = getAppContext().getSmsc().getSmscConfig();
    Collection aliasesStrs = config.getSectionChildShortParamsNames("directives");
    for (Iterator i = aliasesStrs.iterator(); i.hasNext();) {
      String alias = (String) i.next();
      String directive = "";
      try {
        directive = config.getString("directives." + alias);
      } catch (Config.ParamNotFoundException e) {
        logger.error("Internal error: Strange, but parameter \"directives." + alias + "\" disappeared...", e);
      } catch (Config.WrongParamTypeException e) {
        logger.error("Parameter \"directives." + alias + "\" must be string", e);
      }
      aliases.put(alias, directive);
    }
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null)
      return done(request.getParameterMap());
    if (mbCancel != null)
      return cancel();

    return result;
  }

  private int cancel()
  {
    return RESULT_DONE;
  }

  private int done(Map params)
  {
    aliases.clear();
    for (int i = 0; i < alias.length; i++) {
      String aliasName = alias[i];
      String aliasHex = StringEncoderDecoder.encodeHEX(aliasName);
      String directive = ((String[]) params.get("directive_" + aliasHex))[0];
      aliases.put(aliasName, directive);
    }

    if (newAlias != null && newAlias.trim().length() > 0 && newDirective != null && newDirective.trim().length() > 0) {
      aliases.put(newAlias, newDirective);
      newAlias = newDirective = "";
    }

    Config config = getAppContext().getSmsc().getSmscConfig();
    config.removeSection("directives");
    for (Iterator i = aliases.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      config.setString("directives." + (String) entry.getKey(), (String) entry.getValue());
    }

    try {
      appContext.getSmsc().saveSmscConfig(config);
      //todo journal
      journalAppend(SubjectTypes.TYPE_directive, null, Actions.ACTION_MODIFY);
      appContext.getStatuses().setSmscChanged(true);
    } catch (AdminException e) {
      logger.error("Couldn't save new SMSC config", e);
      return error(SMSCErrors.error.smsc.couldntSave);
    }
    return RESULT_DONE;
  }

  public Map getAliases()
  {
    return aliases;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String[] getAlias()
  {
    return alias;
  }

  public void setAlias(String[] alias)
  {
    this.alias = alias;
  }

  public String getNewAlias()
  {
    return newAlias;
  }

  public void setNewAlias(String newAlias)
  {
    this.newAlias = newAlias;
  }

  public String getNewDirective()
  {
    return newDirective;
  }

  public void setNewDirective(String newDirective)
  {
    this.newDirective = newDirective;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
