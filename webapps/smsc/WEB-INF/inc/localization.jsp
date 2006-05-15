<%@ page import="ru.novosoft.smsc.util.LocaleMessages,
                 ru.novosoft.smsc.jsp.SMSCAppContext,
                 javax.servlet.http.HttpServletRequest,
                 java.util.Locale,
                 java.util.ResourceBundle,
                 java.util.MissingResourceException" %><%!
    private HttpServletRequest req = null;

    String getLocJSConfFileName() {
        String result = "/scripts/localization.js";
        if (LocaleMessages.configFile != null)
            try {
                result = LocaleMessages.configFile.getString("conf.jsConfig");
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        return result;
    }

    String getCalendarLocMsgFileName() {
        String result = "/scripts/calendar_msg_" + getLoc().getLanguage() + ".js";
        if (LocaleMessages.configFile != null)
            try {
                result = LocaleMessages.configFile.getString("languages." + getLoc().getLanguage() + ".jsMessages.calendar");
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        return result;
    }

    String getScriptsLocMsgFileName() {
        String result = "/scripts/scripts_msg_" + getLoc().getLanguage() + ".js";
        if (LocaleMessages.configFile != null)
            try {
                result = LocaleMessages.configFile.getString("languages." + getLoc().getLanguage() + ".jsMessages.scripts");
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        return result;
    }

    String getLocImageFileName(String imageName) {
        String result = "/images/" + imageName + "_" + getLoc().getLanguage() + ".gif";
        if (LocaleMessages.configFile != null)
            try {
                result = LocaleMessages.configFile.getString("languages." + getLoc().getLanguage() + ".images." + imageName);
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        return result;
    }

    Locale getLoc() {
        Locale result = null;
        SMSCAppContext appContext = null;
        if (req != null) appContext = (SMSCAppContext) req.getAttribute("appContext");
        if (appContext != null) result = appContext.getUserManager().getPrefs(req.getUserPrincipal()).getLocale();
        if (result == null) result = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
        return result;
    }

    String getLocString(String key) {
        String result = "";
        if (req == null) result = getLocStringWithoutBean(key);
        else {
            SMSCAppContext appCon = (SMSCAppContext) req.getAttribute("appContext");
            if (appCon != null) result = appCon.getLocaleString(req.getUserPrincipal(), key);
            else
                result = getLocStringWithoutBean(key);
        }
        return result;
    }

    String getLocStringWithoutBean(String key) {
        String result = "";
        try {
            ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE));
            result = bundle.getString(key);
        }
        catch (MissingResourceException e) {/*result = "ResourceNotFound: " + key;*/ result = key;
        }
        return result;
    }%><%req = request;%>