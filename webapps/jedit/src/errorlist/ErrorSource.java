/*
 * ErrorSource.java - An error source
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 1999 Slava Pestov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

package errorlist;

//{{{ Imports
import org.gjt.sp.jedit.*;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.Enumeration;
//}}}

/**
 * An error source.
 * @author Slava Pestov
 */
public abstract class ErrorSource implements EBComponent
{
 //{{{ Static part

 //{{{ registerErrorSource() method
 /**
  * Registers an error source.
  * @param errorSource The error source
  */
 public static void registerErrorSource(View view, final ErrorSource errorSource)
 {
  if(errorSource.registered )
  {
    System.out.println("ErrorSource "+errorSource.getName()+" is registered!!!");
    return;
  }
  synchronized(errorSources)
  {
   System.out.println("REGISTERED in registerErrorSource "+errorSource.getName()+" ErrorSource with errors quantity: "+errorSource.getErrorCount());
   errorSources.put(view,errorSource);
   errorSource.registered=true;
   cachedErrorSources = null;
   System.out.println("ErrorSource.registerErrorSource line 53 EditBus.send(new ErrorSourceUpdate(errorSource,ErrorSourceUpdate.ERROR_SOURCE_ADDED,null)) errorSources.size= "+errorSources.size());
   EditBus.send(new ErrorSourceUpdate(errorSource,ErrorSourceUpdate.ERROR_SOURCE_ADDED,null,view, errorSource.getErrorCount()>0 ? false : true));
  }
 } //}}}

  //{{{ registerAndCheckErrorSource() method
 /**
  * Registers an error source.
  * @param errorSource The error source
  */
  public static void registerAndCheckErrorSource(View view, final ErrorSource errorSource)
  {
    if(errorSource.registered)
     return;
  synchronized(errorSources)
  {
   if (errorSources.size()>0) return;
   System.out.println("REGISTERED in registerAndCheckErrorSource "+errorSource.getName()+" ErrorSource with errors quantity: "+errorSource.getErrorCount());
   errorSources.put(view, errorSource);
   errorSource.registered=true;
   cachedErrorSources = null;
   System.out.println("ErrorSource.registerAndCheckErrorSource line 53 EditBus.send(new ErrorSourceUpdate(errorSource,ErrorSourceUpdate.ERROR_SOURCE_ADDED,null)) errorSources.size= "+errorSources.size());
   EditBus.send(new ErrorSourceUpdate(errorSource,ErrorSourceUpdate.ERROR_SOURCE_ADDED,null,view,false));
  }
  } //}}}

  public static void unregisterErrorSource(View view)
  {
    ErrorSource errorSource = (ErrorSource)errorSources.remove(view);
    unregisterErrorSource(view,errorSource);
  }
 //{{{ unregisterErrorSource() method
 /**
  * Unregisters an error source.
  * @param errorSource The error source
  */
 public static void unregisterErrorSource(View view,final ErrorSource errorSource)
 {
    if (errorSource == null ) return;
    /*if(!errorSource.registered)
    {
      System.out.println("erroSource "+ errorSource.getName()+" is unregistered so return from unregisterErrorSource");
      return;
    }  */
    EditBus.removeFromBus(errorSource);

   synchronized(errorSources)
   {
    System.out.println("UNREGISTERED "+errorSource.getName()+" ErrorSource  with errors quantity: "+errorSource.getErrorCount());
    //errorSources.remove(view);
    errorSource.registered=false;
    cachedErrorSources = null;
    System.out.println("ErrorSource.unregisterErrorSource line 95 EditBus.send(new ErrorSourceUpdate(errorSource,ErrorSourceUpdate.ERROR_SOURCE_REMOVED,null)) errorSources.size= "+errorSources.size());
    System.out.println();
    EditBus.send(new ErrorSourceUpdate(errorSource,ErrorSourceUpdate.ERROR_SOURCE_REMOVED,null,view,true));
    synchronized(serviceErrors)
    {
      serviceErrorsClear(view);
    }
  }
 } //}}}
 public static void unregisterByView(final View view)
 {
  synchronized(errorSources)
  {
    errorSources.remove(view);
  }
  unregisterServiceErrors(view);
 }

 private static void unregisterServiceErrors(View view)
 {
  synchronized(serviceErrors)
  {
    //System.out.println("unregisterServiceErrors(view)  ^^^^^^^^^^^^^");
    serviceErrorsClear(view);
    serviceErrors.remove(view);//.getBuffer().getSymlinkPath());
  }
 }

 private static void serviceErrorsClear(View view)
 {
     Hashtable hash = (Hashtable)serviceErrors.get(view);
     if (hash!=null)
     {
       //System.out.println(" hash != null ^^^^^^^^^^^^^");
       //System.out.println(" view = " + view);
       //System.out.println(" view.getBuffer().getSymlinkPath() = " + view.getBuffer().getSymlinkPath());

     // this in not quite correct way for service errors cleaning
     // it would work better if we could extract list of errors from map by path in that way:
     //final ArrayList list = (ArrayList)(hash.get(view.getBuffer().getSymlinkPath()));
     //but view.getBuffer() return null, so we have to iterate over all list from hash  
     for (Enumeration e = hash.elements();e.hasMoreElements();)
     {
       ArrayList list = (ArrayList)e.nextElement();
       if(list != null)
       {
              for(int i = 0; i < list.size(); i++)
              {
                      //System.out.println(" !!!!!!!!sending!!!!!!!!!!!!!!");
                      ErrorSource.Error error = (ErrorSource.Error)list.get(i);
                      ErrorSourceUpdate message = new ErrorSourceUpdate(new DefaultErrorSource("",view), ErrorSourceUpdate.ERROR_REMOVED,error);
                      EditBus.send(message);
              }
       }
       else
       {
         // System.out.println(" list is null!!!!");
       }
     }
    }
    else
    {
       //System.out.println(" hash == null is true _------------");
    }
 }
 //{{{ getErrorSources() method
 /**
  * Returns an array of registered error sources.
  */
 public static ErrorSource[] getErrorSources()
 {
  synchronized(errorSources)
  {
   if(cachedErrorSources == null)
   {
    cachedErrorSources = new ErrorSource[
     errorSources.size()];
    errorSources.values().toArray(cachedErrorSources);
   }
   return cachedErrorSources;
  }
 } //}}}

 //}}}

 public static ErrorSource getErrorSourceByView(View view)
 {
     return (ErrorSource)errorSources.get(view);
 }
 //{{{ Constants
 /**
  * An error.
  */
 public static final int ERROR = 0;

 /**
  * A warning.
  */
 public static final int WARNING = 1;

 //local
 public static final int ERORR_WARNING_LOCAL = 2;

 //remote
 public static final int ERORR_WARNING_REMOTE = 3;
 //}}}

 //{{{ getName() method
 /**
  * Returns a string description of this error source.
  */
 public abstract String getName();
 //}}}

 //{{{ getErrorCount() method
 /**
  * Returns the number of errors in this source.
  */
 public abstract int getErrorCount();
 //}}}

 //{{{ getAllErrors() method
 /**
  * Returns an array of all errors in this error source.
  */
 public abstract Error[] getAllErrors();
 //}}}

 //{{{ getFileErrorCount() method
 /**
  * Returns the number of errors in the specified file.
  * @param path Full path name
  */
 public abstract int getFileErrorCount(String path);
 //}}}

 //{{{ getFileErrors() method
 /**
  * Returns all errors in the specified file.
  * @param path Full path name
  */
 public abstract Error[] getFileErrors(String path);
 //}}}

 //{{{ getLineErrors() method
 /**
  * Returns all errors in the specified line range.
  * @param path The file path
  * @param startLineIndex The line number
  * @param endLineIndex The line number
  * @since ErrorList 1.3
  */
 public abstract ErrorSource.Error[] getLineErrors(String path,
  int startLineIndex, int endLineIndex);
 //}}}

    public static void unregisterAll()
    {
      errorSources =  new HashMap();
      serviceErrors = new Hashtable();
    }

 //{{{ Private members

 // unregistered error sources do not fire events.
 // the console uses this fact to 'batch' multiple errors together
 // for improved performance
 protected boolean registered;

 private static HashMap errorSources = new HashMap();
 protected static Hashtable serviceErrors = new Hashtable();
 private static ErrorSource[] cachedErrorSources;
 //}}}

 //{{{ Error interface
 /**
  * An error.
  */
 public interface Error
 {
  //{{{ getErrorType() method
  /**
   * Returns the error type (error or warning)
   */
  int getErrorType();
  //}}}

  //{{{ getErrorSource() method
  /**
   * Returns the source of this error.
   */
  ErrorSource getErrorSource();
  //}}}

  //{{{ getBuffer() method
  /**
   * Returns the buffer involved, or null if it is not open.
   */
  Buffer getBuffer();
  //}}}

  //{{{ getFilePath() method
  /**
   * Returns the file path name involved.
   */
  String getFilePath();
  //}}}

  //{{{ getFileName() method
  /**
   * Returns just the name portion of the file involved.
   */
  String getFileName();
  //}}}

  //{{{ getLineNumber() method
  /**
   * Returns the line number.
   */
  int getLineNumber();
  //}}}

  //{{{ getStartOffset() method
  /**
   * Returns the start offset.
   */
  int getStartOffset();
  //}}}

  //{{{ getEndOffset() method
  /**
   * Returns the end offset.
   */
  int getEndOffset();
  //}}}

  //{{{ getErrorMessage() method
  /**
   * Returns the error message.
   */
  String getErrorMessage();
  //}}}

  //{{{ getExtraMessages() method
  /**
   * Returns the extra error messages.
   */
  String[] getExtraMessages();
  //}}}
 } //}}}
}
