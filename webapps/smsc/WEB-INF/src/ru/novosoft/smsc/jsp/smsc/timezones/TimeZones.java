package ru.novosoft.smsc.jsp.smsc.timezones;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.util.Collection;
import java.util.LinkedList;
import java.util.TimeZone;

public class TimeZones {
    private static final String TIMEZONES_CONFIG = "core.timezones_config";
    private static final String TIMEZONES_DEFAULT = "default_timezone";
    public static final String TIMEZONES_MASKS_PREFIX = "masks";
    public static final String TIMEZONES_SUBJECTS_PREFIX = "subjects";

    private static TimeZones instance = null;

    public static TimeZones getInstance(SMSCAppContext appContext) throws Throwable {
        return instance == null ? instance = new TimeZones(appContext) : instance;
    }

    private Category logger = Category.getInstance(this.getClass());
    private SMSCAppContext appContext = null;
    private Config config = null;

    public TimeZones(SMSCAppContext appContext) throws Throwable {
        try {
            this.appContext = appContext;
            config = getTimeZonesConfig();
            if (config == null) {
                throw new AdminException(SMSCErrors.error.smsc.timezones.couldntGetTimezoneConfig);
            }
        }
        catch (Throwable e) {
            logger.debug("Couldn't get timezone config", e);
            throw e;
        }
    }

    public String getDefaultTimeZone() {
        try {
            return config.getString(TIMEZONES_DEFAULT);
        } catch (Config.ParamNotFoundException e) {
            return "";
        } catch (Config.WrongParamTypeException e) {
            logger.error("default timezone \"" + TIMEZONES_DEFAULT + "\" is not string");
            return "";
        }
    }

    public void setDefaultTimeZone(String defaultReshedule) {
        config.setString(TIMEZONES_DEFAULT, defaultReshedule);
    }

    private Config getTimeZonesConfig() throws Throwable {
        Config smscConfig = appContext.getSmsc().getSmscConfig();
        if (smscConfig == null) {
            throw new AdminException(SMSCErrors.error.smsc.couldntGetConfig);
        }
        String configName = smscConfig.getString(TIMEZONES_CONFIG);
        File confFile = new File(WebAppFolders.getWebappFolder(), configName);
        return new Config(confFile);
    }

    public void putMask(String mask, String timezone) throws AdminException {
        new Mask(mask); //проверка валидности маски
        putItem(TIMEZONES_MASKS_PREFIX + '.' + mask, mask, timezone);
    }

    public void removeMask(String mask) {
        remove(TIMEZONES_MASKS_PREFIX + '.' + mask);
    }

    public void putSubject(String subj, String timezone) throws AdminException {
        putItem(TIMEZONES_SUBJECTS_PREFIX + '.' + subj, subj, timezone);
    }

    public void removeSubject(String subj) {
        remove(TIMEZONES_SUBJECTS_PREFIX + '.' + subj);
    }

    public TimeZone getMaskTimeZone(String mask) throws AdminException {
        if (isMaskPresent(mask)) {
            return getTimeZone(TIMEZONES_MASKS_PREFIX + '.' + mask);
        } else
            return TimeZone.getTimeZone(getDefaultTimeZone());
    }

    public TimeZone getSubjectTimeZone(String subj) throws AdminException {
        if (isSubjectPresent(subj)) {
            return getTimeZone(TIMEZONES_SUBJECTS_PREFIX + '.' + subj);
        } else
            return TimeZone.getTimeZone(getDefaultTimeZone());
    }

    public String[] getAvailableIDs() {
        return TimeZone.getAvailableIDs();
    }

    public void save() throws Exception {
        config.save();
    }

    public Collection getMasks() {
        return getItems(TIMEZONES_MASKS_PREFIX);
    }

    public Collection getSubjects() {
        return getItems(TIMEZONES_SUBJECTS_PREFIX);
    }

    protected boolean isMaskPresent(String mask) {
        return isPresent(TIMEZONES_MASKS_PREFIX + '.' + mask);
    }

    protected boolean isSubjectPresent(String subj) {
        return isPresent(TIMEZONES_SUBJECTS_PREFIX + '.' + subj);
    }

    private Collection getItems(String key) {
        if (config == null) return new LinkedList();
        Collection c = config.getSectionChildShortParamsNames(key);
        if (c != null) return new SortedList(c);
        else return new LinkedList();
    }

    private TimeZone getTimeZone(String key) throws AdminException {
        try {
            String str = config.getString(key);
            str = str.substring(str.indexOf(",") + 1);
            TimeZone tmz = TimeZone.getTimeZone(str);
            if (tmz.getRawOffset() == 0 && !str.equals("GMT")) throw new AdminException("Incorrect timezone in config: "+str);
            return tmz;
        } catch (Config.ParamNotFoundException e) {
            return TimeZone.getTimeZone(getDefaultTimeZone());
        } catch (Config.WrongParamTypeException e) {
            return TimeZone.getTimeZone(getDefaultTimeZone());
        }
    }

    private void remove(String key) {
        if (config == null) return;
        config.removeParam(key);
    }

    private boolean isPresent(String key) {
        return config.getParameter(key) != null;
    }

    private void putItem(String key, String name, String value) throws AdminException {
        if (config == null) return;
        if (name != null && !name.equals("") && value != null && !value.equals("")) {
            TimeZone tz = TimeZone.getTimeZone(value);
            if (tz.getRawOffset() == 0 && !value.equals("GMT")) throw new AdminException("not valid timezone: " + value);
            config.setString(key, Integer.toString(TimeZone.getTimeZone(value).getRawOffset() / 60000) + "," + value);
        }
    }
}
