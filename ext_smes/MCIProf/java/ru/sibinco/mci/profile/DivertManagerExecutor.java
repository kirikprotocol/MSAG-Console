package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:59:18
 * To change this template use File | Settings | File Templates.
 */
public class DivertManagerExecutor extends DivertManagerState implements Executor
{
  private static Category logger = Category.getInstance(DivertManagerExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueOff = null;
  protected String valueService = null;
  protected String valueVoicemail = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      pageFormat = new MessageFormat(Transliterator.translit(divertBundle.getString(Constants.PAGE_INFO)));
      valueOff = Transliterator.translit(divertBundle.getString(Constants.VALUE_OFF));
      valueService = Transliterator.translit(divertBundle.getString(Constants.VALUE_SERVICE));
      valueVoicemail = Transliterator.translit(divertBundle.getString(Constants.VALUE_VOICEMAIL));
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }

  }

  protected String getValue(String option)
  {
    if (option == null || option.equalsIgnoreCase(Constants.OFF)) return valueOff;
    else if (option.equalsIgnoreCase(Constants.SERVICE)) return valueService;
    else if (option.equalsIgnoreCase(Constants.VOICEMAIL)) return valueVoicemail;
    return Transliterator.translit(option);
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    DivertInfo info = getDivertInfo(state);
    Object[] args = new Object[] {getValue(info.getBusy()), getValue(info.getAbsent()),
                                  getValue(info.getNotavail()), getValue(info.getUncond())};
    Message resp = new Message();
    resp.setMessageString(pageFormat.format(args));
    state.setAttribute(Constants.ATTR_MAIN, Constants.ATTR_MAIN);
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
