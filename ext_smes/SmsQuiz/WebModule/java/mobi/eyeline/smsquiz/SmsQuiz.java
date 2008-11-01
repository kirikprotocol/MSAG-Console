package mobi.eyeline.smsquiz;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.jsp.SMSCAppContext;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class SmsQuiz {

  private boolean running = false;
  private Config config;
  private SMSCAppContext appContext;

  public SmsQuiz(SMSCAppContext appContext, Config config) {
    this.config = config;
    this.appContext = appContext;
  }

  public boolean isRunning() {
    return running;
  }

  public void setRunning(boolean running) {
    this.running = running;
  }
}
