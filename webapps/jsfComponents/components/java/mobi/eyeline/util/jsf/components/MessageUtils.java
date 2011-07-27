package mobi.eyeline.util.jsf.components;

import com.sun.corba.se.impl.monitoring.MonitoredObjectImpl;

import javax.el.ValueExpression;
import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import java.text.MessageFormat;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * User: artem
 * Date: 27.07.11
 */
public class MessageUtils {

  private static final String DETAIL_SUFFIX = "_detail";

  public static FacesMessage getErrorMessage(FacesContext facesContext, String messageId, String ... args) {
    return getMessage(facesContext, facesContext.getViewRoot().getLocale(), FacesMessage.SEVERITY_ERROR, messageId, args);
  }

  public static String getMessageString(FacesContext facesContext, String messageId, String ... args) {
    return getMessageString(facesContext, facesContext.getViewRoot().getLocale(), messageId, args);
  }

  public static String getMessageString(FacesContext facesContext, Locale locale, String messageId, String ... args) {
    String result;

    ResourceBundle appBundle = getApplicationBundle(facesContext, locale);
    result = getBundleString(appBundle, messageId, args);

    if (result == null) {
      ResourceBundle defBundle = getDefaultBundle(facesContext, locale);
      result = getBundleString(defBundle, messageId, args);
    }

    return result;
  }

  public static FacesMessage getMessage(FacesContext facesContext, Locale locale, FacesMessage.Severity severity, String messageId, String ... args) {
    ResourceBundle appBundle;
    ResourceBundle defBundle;
    String summary;
    String detail;

    appBundle = getApplicationBundle(facesContext, locale);
    summary = getBundleString(appBundle, messageId, args);
    if (summary != null) {
      detail = getBundleString(appBundle, messageId + DETAIL_SUFFIX, args);

    } else {
      defBundle = getDefaultBundle(facesContext, locale);
      summary = getBundleString(defBundle, messageId, args);
      if (summary != null) {
        detail = getBundleString(defBundle, messageId + DETAIL_SUFFIX, args);
      } else {
        //Try to find detail alone
        detail = getBundleString(appBundle, messageId + DETAIL_SUFFIX, args);
        if (detail != null) {
          summary = null;
        } else {
          detail = getBundleString(defBundle, messageId + DETAIL_SUFFIX, args);
          if (detail != null) {
            summary = null;
          } else {
            facesContext.getExternalContext().log("No message with id " + messageId + " found in any bundle");
            return new FacesMessage(severity, messageId, null);
          }
        }
      }
    }

    return new FacesMessage(severity, summary, detail);
  }

  private static String getBundleString(ResourceBundle bundle, String key, String[] args) {
    try {
      String res = (bundle == null) ? null : bundle.getString(key);
      if (res != null && args != null)
        res = MessageFormat.format(res, args);
      return res;
    } catch (MissingResourceException e) {
      return null;
    }
  }


  private static ResourceBundle getApplicationBundle(FacesContext facesContext, Locale locale) {
    String bundleName = facesContext.getApplication().getMessageBundle();
    return bundleName != null ? getBundle(facesContext, locale, bundleName) : null;
  }

  private static ResourceBundle getDefaultBundle(FacesContext facesContext, Locale locale) {
    return getBundle(facesContext, locale, "mobi.eyeline.util.jsf.components.Messages");
  }

  private static ResourceBundle getBundle(FacesContext facesContext, Locale locale, String bundleName) {
    try {
      //First we try the JSF implementation class loader
      return ResourceBundle.getBundle(bundleName, locale, facesContext.getClass().getClassLoader());
    } catch (MissingResourceException ignore1) {
      try {
        //Next we try the JSF API class loader
        return ResourceBundle.getBundle(bundleName, locale, MessageUtils.class.getClassLoader());
      } catch (MissingResourceException ignore2) {
        try {
          //Last resort is the context class loader
          return ResourceBundle.getBundle(bundleName, locale, Thread.currentThread().getContextClassLoader());
        } catch (MissingResourceException damned) {
          facesContext.getExternalContext().log("resource bundle " + bundleName + " could not be found");
          return null;
        }
      }
    }
  }


}
