package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;


import com.eyeline.utils.config.ConfigException;

import java.io.IOException;

/**
 * author: alkhal
 */
public class FileStatsException extends Exception{
    public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT,ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_IO, ERROR_CONFIG}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    FileStatsException() {
    }

    FileStatsException(String message) {
      super(message);
    }

    FileStatsException(String message, Throwable cause) {
      super(message, cause);
    }

    FileStatsException(Throwable cause) {
      super(cause);
    }
    FileStatsException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }

    FileStatsException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    FileStatsException(String message, IOException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_IO;
    }

    ErrorCode getErrorCode() {
        return errorCode;
    }
}
