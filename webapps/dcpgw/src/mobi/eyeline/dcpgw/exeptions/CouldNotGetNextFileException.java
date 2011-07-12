package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 11.07.11
 * Time: 11:48
 * To change this template use File | Settings | File Templates.
 */
public class CouldNotGetNextFileException extends Exception{

     public CouldNotGetNextFileException(String message){
         super(message);
     }

     public CouldNotGetNextFileException(String message, Throwable cause){
         super(message, cause);
     }

     public CouldNotGetNextFileException(Throwable cause){
         super(cause);
     }
}
