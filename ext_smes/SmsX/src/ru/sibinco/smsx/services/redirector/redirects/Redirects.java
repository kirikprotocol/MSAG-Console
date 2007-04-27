package ru.sibinco.smsx.services.redirector.redirects;

import org.apache.log4j.Category;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.services.redirector.RedirectorService;
import ru.sibinco.smsx.services.redirector.redirects.parsers.RedirectsParser;
import ru.sibinco.smsx.utils.Service;

import java.io.File;
import java.net.URI;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Matcher;

/**
 * User: artem
 * Date: 24.01.2007
 */

public class Redirects {

  private static final Category log = Category.getInstance(Redirects.class);

  private static List redirects;
  private static final Object redirectsLock = new Object();


  public static List getRedirects() {
    return redirects;
  }

  public static Redirect getRedirectByMessage(String message) {
    Redirect redirect;
    Matcher matcher;
    for (Iterator iter = redirects.iterator(); iter.hasNext();) {
      redirect = (Redirect)iter.next();
      matcher = redirect.getFormat().matcher(message.trim());
      if (matcher.matches())
        return redirect;
    }
    return null;
  }

  public static void init(){
    try {
      redirects = RedirectsParser.parseFile(RedirectorService.Properties.REDIRECTS_XML);
      new RedirectsReloader().startService();
    } catch (Exception e) {
      throw new InitializationException("Can't load redirects. " + e.getMessage());
    }
  }

  private static class RedirectsReloader extends Service {
    private boolean errorOccured = false;
    private long lastModifiedTime = -1;

    RedirectsReloader() {
      super(log);
    }

    public synchronized void iterativeWork() {
      try {
        wait((!errorOccured) ? RedirectorService.Properties.RELOAD_REDIRECTS_INTERVAL : 15000);
      } catch (InterruptedException e) {
        log.error("Interrupted", e);
      }

      final File tf = new File(URI.create(ClassLoader.getSystemResource(RedirectorService.Properties.REDIRECTS_XML).toString()));

      if (lastModifiedTime != tf.lastModified()) {
        try {
          final List newRedirects = RedirectsParser.parseFile(RedirectorService.Properties.REDIRECTS_XML);

          if (newRedirects != null) {
            log.info("Redirects have been reloaded");
            errorOccured = false;
            lastModifiedTime = tf.lastModified();

            synchronized(redirectsLock) {
              redirects = newRedirects;
            }
          } else
            log.info("Can't read redirects. Error.");

        } catch (Throwable e) {
          log.error("Error reading redirects!!!", e);
          errorOccured = true;
        }
      }
    }
  }
}
