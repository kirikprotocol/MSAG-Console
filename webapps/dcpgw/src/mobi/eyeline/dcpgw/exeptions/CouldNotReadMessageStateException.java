package mobi.eyeline.dcpgw.exeptions;

import mobi.eyeline.informer.admin.AdminException;

public class CouldNotReadMessageStateException extends AdminException{

    public CouldNotReadMessageStateException(String key, Throwable cause, String... args) {
        super(key, cause, args);
    }

}
