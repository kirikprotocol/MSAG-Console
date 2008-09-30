package mobi.eyeline.smsquiz.storage;

import com.eyeline.utils.config.ConfigException;

import java.sql.SQLException;



public class StorageException extends Exception{

    public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT, ERROR_DB, ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_CONFIG}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;
    
    public StorageException() {
    }

    public StorageException(String message) {
      super(message);
    }

    public StorageException(String message, Throwable cause) {
      super(message, cause);
    }

    public StorageException(Throwable cause) {
      super(cause);
    }
    public StorageException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }
    public StorageException(String message, SQLException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_DB;
    }
    public StorageException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }

    public ErrorCode getErrorCode() {
        return errorCode;
    }


}
