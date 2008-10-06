package mobi.eyeline.smsquiz.replystats.statsfile;


import com.eyeline.utils.config.ConfigException;

import java.io.IOException;

/**
 * author: alkhal
 */
public class FileStatsException extends Exception{
    public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT,ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_IO, ERROR_CONFIG}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    public FileStatsException() {
    }

    public FileStatsException(String message) {
      super(message);
    }

    public FileStatsException(String message, Throwable cause) {
      super(message, cause);
    }

    public FileStatsException(Throwable cause) {
      super(cause);
    }
    public FileStatsException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }

    public FileStatsException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    public FileStatsException(String message, IOException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_IO;
    }

    public ErrorCode getErrorCode() {
        return errorCode;
    }
}
