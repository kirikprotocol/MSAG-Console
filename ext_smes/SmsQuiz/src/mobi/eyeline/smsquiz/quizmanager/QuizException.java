package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.config.ConfigException;

import java.io.IOException;

import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;

/**
 * author: alkhal
 */
public class QuizException extends Exception{

   public enum ErrorCode {ERROR_DEFAULT, ERROR_INIT,ERROR_NOT_INITIALIZED, ERROR_WRONG_REQUEST, ERROR_IO, ERROR_CONFIG, ERROR_DISTRIBUTION, ERROR_REPLY_STATS}
    private ErrorCode errorCode = ErrorCode.ERROR_DEFAULT;

    public QuizException() {
    }

    public QuizException(String message) {
      super(message);
    }

    public QuizException(String message, Throwable cause) {
      super(message, cause);
    }

    public QuizException(Throwable cause) {
      super(cause);
    }
    public QuizException(String message, ErrorCode errorCode){
        super(message);
        this.errorCode = errorCode;
    }

    public QuizException(String message, ConfigException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_CONFIG;
    }
    public QuizException(String message, IOException preException){
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_IO;
    }
    public QuizException(String message, DistributionException preException) {
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_DISTRIBUTION;       
    }
    public QuizException(String message, ReplyDataSourceException preException) {
        super(message, preException);
        this.errorCode = ErrorCode.ERROR_REPLY_STATS;         
    }
    public ErrorCode getErrorCode() {
        return errorCode;
    }
}
