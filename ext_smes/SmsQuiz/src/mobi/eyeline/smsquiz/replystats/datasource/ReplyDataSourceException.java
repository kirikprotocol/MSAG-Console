package mobi.eyeline.smsquiz.replystats.datasource;

import com.eyeline.utils.config.ConfigException;

import java.io.IOException;

import mobi.eyeline.smsquiz.replystats.statsfile.FileStatsException;

/**
 * author: alkhal
 */
public class ReplyDataSourceException extends Exception{
    public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT,ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_IO, ERROR_CONFIG, ERROR_STATS_FILE}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    public ReplyDataSourceException() {
    }

    public ReplyDataSourceException(String message) {
      super(message);
    }

    public ReplyDataSourceException(String message, Throwable cause) {
      super(message, cause);
    }

    public ReplyDataSourceException(Throwable cause) {
      super(cause);
    }
    public ReplyDataSourceException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }

    public ReplyDataSourceException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    public ReplyDataSourceException(String message, IOException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_IO;
    }
    public ReplyDataSourceException(String message, FileStatsException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_STATS_FILE;
    }
    public ErrorCode getErrorCode() {
        return errorCode;
    }
}
