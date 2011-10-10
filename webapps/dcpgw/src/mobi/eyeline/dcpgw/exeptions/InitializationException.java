package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 03.06.11
 * Time: 10:05
 */
public class InitializationException extends Exception{

     public InitializationException(String message){
         super(message);
     }

     public InitializationException(Throwable cause){
         super(cause);
     }
}

