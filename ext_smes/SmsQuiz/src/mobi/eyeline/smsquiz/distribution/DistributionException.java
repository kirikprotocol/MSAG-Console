package mobi.eyeline.smsquiz.distribution;

import com.eyeline.utils.config.ConfigException;

import java.io.IOException;

/**
 * author: alkhal
 */
public class DistributionException extends Exception{
   public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT,ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_IO, ERROR_CONFIG}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    public DistributionException() {
    }

    public DistributionException(String message) {
      super(message);
    }

    public DistributionException(String message, Throwable cause) {
      super(message, cause);
    }

    public DistributionException(Throwable cause) {
      super(cause);
    }
    public DistributionException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }

    public DistributionException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    public DistributionException(String message, IOException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_IO;
    }

    public ErrorCode getErrorCode() {
        return errorCode;
    }
}
