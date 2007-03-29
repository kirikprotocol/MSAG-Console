package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.*;

import java.text.MessageFormat;
import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 22.10.2004
 * Time: 17:44:04
 */
public class ExitExecutor extends AbstractExecutor
{
  private static Category logger = Category.getInstance(MainExecutor.class);

  private MessageFormat pageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
    try {
      pageFormat   =  new MessageFormat(systemBundle.getString(Constants.PAGE_EXIT));
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    final String msg = pageFormat.format(new Object[] {});
    return new ExecutorResponse(translit(msg), true);
  }

}
