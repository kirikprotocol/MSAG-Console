package ru.sibinco.mci;

import org.apache.log4j.Category;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundleException;
import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.MessageData;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 23.12.2004
 * Time: 15:57:58
 */
public class ShortcutHelpExecutor extends EntryExecutor
{
  private static Category logger = Category.getInstance(ShortcutHelpExecutor.class);

  protected ScenarioResourceBundle helpBundle = null;

  private String pageShortcuts = null;
  private String pageManual = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
    try {
      helpBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_HELP);
      pageShortcuts = helpBundle.getString(Constants.PAGE_SHORTCUTS);
      MessageFormat pageFormat = new MessageFormat(helpBundle.getString(Constants.PAGE_MAN_SHORTCUTS));
      if (strategy == Constants.RELEASE_REDIRECT_STRATEGY ||
          strategy == Constants.RELEASE_REDIRECT_RULES_STRATEGY)
        pageManual = pageFormat.format(new Object[] {helpBundle.getString(Constants.PAGE_MAN_REDIRECT)});
      else if (strategy == Constants.RELEASE_MIXED_STRATEGY ||
               strategy == Constants.RELEASE_PREFIXED_STRATEGY)
        pageManual = pageFormat.format(new Object[] {helpBundle.getString(Constants.PAGE_MAN_PREFIXED)});
      else
        throw new ScenarioInitializationException("Strategy '"+strategy+"' is invalid");
    } catch (ScenarioResourceBundleException e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }
  
  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message info = new Message();
    info.setMessageString(translit(pageShortcuts));
    MessageData resp = new MessageData(info);    
    Message man = new Message(); man.setMessageString(pageManual);
    MessageData manual = new MessageData(man);
    return new ExecutorResponse(new MessageData[]{resp, manual}, true);
  }

}
