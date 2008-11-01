package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;

import java.util.List;

import mobi.eyeline.smsquiz.SmsQuizContext;
import mobi.eyeline.smsquiz.SmsQuiz;

import javax.servlet.http.HttpServletRequest;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class SmsQuizBean extends PageBean {


  protected SmsQuizContext smsQuizContext;
  protected SmsQuiz smsQuiz = null;
  protected Config config;

  protected String smeId = "SmsQuiz";
  protected boolean smeRunning = false;

protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      smsQuizContext = SmsQuizContext.getInstance(appContext, smeId);
      smsQuiz = smsQuizContext.getSmsQuiz();
      smeRunning = smsQuiz.isRunning();
      config = smsQuizContext.getConfig();
    } catch (Throwable e) {
      logger.error("Couldn't get SmsQuiz config", e);
      return error("infosme.error.config_load", e);
    }

    return result;
  }

  public int process(HttpServletRequest request){
    return super.process(request);
  }


  protected Config getConfig()
  {
    return config;
  }

  public SmsQuizContext getSmsQuizContext()
  {
    return smsQuizContext;
  }

  public SmsQuiz getSmsQuiz()
  {
    return smsQuiz;
  }

  public boolean isSmeRunning()
  {
    return smeRunning;
  }

  public String getSmeId()
  {
    return smeId;
  }



}
