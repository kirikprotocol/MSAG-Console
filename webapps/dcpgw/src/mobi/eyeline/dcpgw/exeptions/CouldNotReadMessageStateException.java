package mobi.eyeline.dcpgw.exeptions;

import mobi.eyeline.informer.admin.AdminException;

public class CouldNotReadMessageStateException extends AdminException{
    public CouldNotReadMessageStateException(String bundleName, String key, String... args) {
        super(bundleName, key, args);
    }

    public CouldNotReadMessageStateException(String bundleName, String key, Throwable cause, String... args) {
        super(bundleName, key, cause, args);
    }

    public CouldNotReadMessageStateException(String key, Throwable cause, String... args) {
        super(key, cause, args);
    }

    public CouldNotReadMessageStateException(String key, String... args) {
        super(key, args);
    }
}
