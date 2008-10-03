package mobi.eyeline.smsquiz.replystats;


import com.eyeline.utils.config.ConfigException;

import java.io.IOException;

/**
 * author: alkhal
 */
public class ReplyStatsException extends Exception{
    public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT,ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_IO, ERROR_CONFIG}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    public ReplyStatsException() {
    }

    public ReplyStatsException(String message) {
      super(message);
    }

    public ReplyStatsException(String message, Throwable cause) {
      super(message, cause);
    }

    public ReplyStatsException(Throwable cause) {
      super(cause);
    }
    public ReplyStatsException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }

    public ReplyStatsException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    public ReplyStatsException(String message, IOException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_IO;
    }

    public ErrorCode getErrorCode() {
        return errorCode;
    }
}
