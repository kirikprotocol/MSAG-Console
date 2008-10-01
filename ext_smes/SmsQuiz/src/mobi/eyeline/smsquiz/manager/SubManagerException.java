package mobi.eyeline.smsquiz.manager;

import com.eyeline.utils.config.ConfigException;
import mobi.eyeline.smsquiz.storage.StorageException;

/**
 * author: alkhal
 */
public class SubManagerException extends Exception{

    public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT, ERROR_DB, ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_CONFIG}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    public SubManagerException() {
    }

    public SubManagerException(String message) {
      super(message);
    }

    public SubManagerException (String message, Throwable cause) {
      super(message, cause);
    }

    public SubManagerException (Throwable cause) {
      super(cause);
    }
    public SubManagerException (String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }
    public SubManagerException (String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    public SubManagerException (String message, StorageException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_DB;
    }
    public ErrorCode getErrorCode() {
        return errorCode;
    }

}
