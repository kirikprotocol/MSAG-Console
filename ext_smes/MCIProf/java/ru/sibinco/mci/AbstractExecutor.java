package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.Executor;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.util.Transliterator;
import org.apache.log4j.Category;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by Serge Lugovoy
 * Date: Mar 28, 2007
 * Time: 2:44:39 PM
 */
public abstract class AbstractExecutor implements Executor {
  private static Category logger = Category.getInstance(AbstractExecutor.class);
  protected ScenarioResourceBundle systemBundle = null;
  protected boolean needTranslit = true;

  public void init(Properties properties) throws ScenarioInitializationException {
    try {
      systemBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
      String ss = systemBundle.getString(Constants.TRANSLIT);
      if( ss != null && ss.equalsIgnoreCase("off") ) needTranslit = false;
    } catch (Exception e) {
      final String err = "Executor init error"; logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  protected String translit(String msg) {
    if( needTranslit ) return Transliterator.translit(msg);
    else return msg;
  }
}
