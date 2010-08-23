package ru.novosoft.smsc.admin.journal;

public abstract class Actions {
    public static final byte ACTION_UNKNOWN = 0;
    public static final byte ACTION_ADD = 1;
    public static final byte ACTION_DEL = 2;
    public static final byte ACTION_MODIFY = 3;
    public static final byte ACTION_LOCK = 4;
    public static final byte ACTION_UNLOCK = 5;
    public static final byte ACTION_RESTORE = 6;
    public static final byte ACTION_LOAD = 7;
    public static final byte ACTION_VIEW = 8;
    public static final byte ACTION_EXPORT = 9;
    public static final byte ACTION_SAVE = 10;
    public static final byte ACTION_DISABLE = 11;
    public static final byte ACTION_ENABLE = 12;
    public static final byte ACTION_ARCHIVATE = 13;

    public static final String actionToString(byte action) {
        switch (action) {
            case ACTION_ADD:
                return "add";
            case ACTION_DEL:
                return "delete";
            case ACTION_MODIFY:
                return "modify";
            case ACTION_LOCK:
                return "lock";
            case ACTION_UNLOCK:
                return "unlock";
            case ACTION_RESTORE:
                return "restore";
            case ACTION_LOAD:
                return "load";
            case ACTION_VIEW:
                return "view";
            case ACTION_EXPORT:
                return "export";
            case ACTION_SAVE:
                return "save";
            case ACTION_DISABLE:
                return "disable";
            case ACTION_ENABLE:
                return "enable";
            default:
                return "unknown";
        }
    }

    public static final byte stringToAction(String action) {
        if ("add".equals(action))
            return ACTION_ADD;
        if ("delete".equals(action))
            return ACTION_DEL;
        if ("modify".equals(action))
            return ACTION_MODIFY;
        if ("lock".equals(action))
            return ACTION_LOCK;
        if ("unlock".equals(action))
            return ACTION_UNLOCK;
        if ("restore".equals(action))
            return ACTION_RESTORE;
        if ("load".equals(action))
            return ACTION_LOAD;
        if ("view".equals(action))
            return ACTION_VIEW;
        if ("export".equals(action))
            return ACTION_EXPORT;
        if ("save".equals(action))
            return ACTION_SAVE;
        if ("disable".equals(action))
            return ACTION_DISABLE;
        if ("enable".equals(action))
            return ACTION_ENABLE;
        return ACTION_UNKNOWN;
    }
}