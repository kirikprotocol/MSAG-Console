package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 09.06.11
 * Time: 10:48
 */
public class UpdateConfigurationException extends Exception{

     public UpdateConfigurationException(String message){
         super(message);
     }

     public UpdateConfigurationException(String message, Throwable cause){
         super(message, cause);
     }

     public UpdateConfigurationException(Throwable cause){
         super(cause);
     }
}
