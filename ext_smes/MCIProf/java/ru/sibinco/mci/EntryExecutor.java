package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.*;

import java.util.Properties;
import java.io.InputStream;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 14.12.2004
 * Time: 15:51:35
 * To change this template use File | Settings | File Templates.
 */
public class EntryExecutor implements Executor
{
  private static Category logger = Category.getInstance(EntryExecutor.class);

  protected String jumpPage = null;
  protected String mciSmeReasons = null;
  protected int    strategy = 0;

  public void loadup() throws ScenarioInitializationException
  {
    InputStream is = this.getClass().getClassLoader().getResourceAsStream(Constants.MCI_PROF_MSC_FILE);
    if (is == null)
      throw new ScenarioInitializationException("Failed to locate MCISme properties file");

    try {
      Properties properties = new Properties();
      properties.load(is);
      String strategyStr = properties.getProperty("MSC.mcisme.strategy");
      strategy = ((strategyStr != null && strategyStr.length() > 0)) ?
          Integer.decode(strategyStr).intValue() : Constants.RELEASE_REDIRECT_STRATEGY;
      mciSmeReasons = properties.getProperty("MSC.mcisme.reasons");
    } catch(Exception e) {
      final String err = "Failed to load MCISme properties";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    } finally {
      try { is.close(); } catch (Throwable th) {}
    }
  }
  public void init(Properties initProperties) throws ScenarioInitializationException
  {
    this.loadup();

    if (strategy == Constants.RELEASE_MIXED_STRATEGY || strategy == Constants.RELEASE_PREFIXED_STRATEGY)
      jumpPage = initProperties.getProperty(Constants.PAGE_PREFIXED_STRATEGY);
    else if (strategy == Constants.RELEASE_REDIRECT_STRATEGY)
      jumpPage = initProperties.getProperty(Constants.PAGE_REDIRECT_STRATEGY);
    else
      throw new ScenarioInitializationException("Invalid MCISme strategy '"+strategy+"' defined");
    if (jumpPage == null)
      throw new ScenarioInitializationException("Invalid init parameters passed to executor. Jump page is undefined");
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    state.setAttribute(Constants.ATTR_STRATEGY, new Integer(strategy));
    state.setAttribute(Constants.ATTR_REASONS, mciSmeReasons);
    return new ExecutorResponse(new Jump(jumpPage, state.getMessageString()));
  }
}
