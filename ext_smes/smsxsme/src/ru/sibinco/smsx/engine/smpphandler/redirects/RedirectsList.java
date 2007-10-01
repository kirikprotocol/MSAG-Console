package ru.sibinco.smsx.engine.smpphandler.redirects;

import com.eyeline.sme.utils.ds.DataSourceException;

import java.util.Collection;
import java.util.Iterator;
import java.util.regex.Matcher;

import ru.sibinco.smsx.InitializationException;

/**
 * User: artem
 * Date: 16.07.2007
 */

public class RedirectsList {
  private Collection redirects;
  private boolean lock = true;
  private int counter;

  private Object lockWait = new Object();
  private Object reloadWait = new Object();

  private final RedirectsDataSource ds;

  public RedirectsList(String redirectsFile) {
    ds = new RedirectsDataSource(redirectsFile);
    try {
      reloadRedirects(ds.getRedirects());
    } catch (DataSourceException e) {
      throw new InitializationException(e);
    }
  }

  public Redirect getRedirectByMessage(String message) {
    if (lock) {
      synchronized(lockWait) {
        try {
          lockWait.wait();
        } catch (InterruptedException e) {
        }
      }
    }

    try {
      counter++;

      Redirect redirect;
      Matcher matcher;
      for (Iterator iter = redirects.iterator(); iter.hasNext();) {
        redirect = (Redirect)iter.next();
        matcher = redirect.getFormat().matcher(message.trim());
        if (matcher.matches())
          return redirect;
      }

      return null;

    } finally {
      counter--;
      synchronized(reloadWait) {
        reloadWait.notifyAll();
      }
    }
  }

  void reloadRedirects(Collection newRedirects) {
    lock = true;
    while(counter > 0) {
      synchronized(reloadWait) {
        try {
          reloadWait.wait();
        } catch (InterruptedException e) {
        }
      }
    }

    redirects = newRedirects;

    lock = false;
    synchronized(lockWait) {
      lockWait.notifyAll();
    }
  }
}
