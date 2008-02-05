package ru.sibinco.scag.backend.installation;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.beans.rules.applet.MiscUtilities;

import java.util.Arrays;
import java.io.*;

import org.apache.log4j.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 05.07.2006
 * Time: 17:25:13
 * To change this template use File | Settings | File Templates.
 */
public class HSDaemon {
  private Logger logger = Logger.getLogger(this.getClass());
  private static final boolean DEBUG = false;

  private static final String typeSingle = "single";
  private static final String typeHS = "HS";
  private static final String types[] = new String[]{typeSingle,typeHS};

  public static final byte UPDATEORADD = 0;
  public static final byte REMOVE = 1;
  public static final byte operationTypes[] = new byte[]{UPDATEORADD,REMOVE};

  private String type;
  private String mirrorPath;
  public HSDaemon(final String type, final String mirrorPath) throws SibincoException {
    checkType(type);
    this.type = type;
    this.mirrorPath = mirrorPath;
  }

  public void store(final File configFile) throws SibincoException {
    doOperation(configFile,UPDATEORADD,new CommonSaver(configFile));
  }

  public void store(final File folder, final String fileName) throws SibincoException {
    store(new File(folder, fileName));
  }

  public void store(final File configFile, SavingStrategy saver) throws SibincoException {
    doOperation(configFile,UPDATEORADD,saver);
  }
//1
  public void doOperation(final File configFile, final byte operationType) throws SibincoException {
      logger.info("HSDaemon:doOperation()1");
    doOperation(configFile, operationType, new CommonSaver(configFile));
  }
//2
  private synchronized void doOperation(final File configFile, final byte operationType, final SavingStrategy saver) throws SibincoException {
    logger.info("HSDaemon:doOperation()2");
    if (type.equals(typeSingle)){
        logger.info("HSDaemon:doOperation()2:typeSingle");
        return;
    }
    else if (type.equals(typeHS)) {
      logger.info("HSDaemon:doOperation()2:typeHS");
      checkOperationType(operationType);
      File mirrorFile = getMirrorFile(configFile);
      switch(operationType)
      {
        case UPDATEORADD:
          logger.info("HSDaemon:doOperation()2:typeHS:UPDATEORADD");
          saver.storeToMirror(mirrorFile);
          break;
       case REMOVE:
          logger.info("HSDaemon:doOperation()2:typeHS:REMOVE");
          remove(mirrorFile);
          break;
      }
    }
  }

  private File getMirrorFile(final File configFile) {
    if (!DEBUG)
        return new File(mirrorPath+configFile.getAbsolutePath());
    else {
      //compose path for debugging
      File debugMirrorFile = new File(configFile.getParent() + mirrorPath + configFile.getName());
      return debugMirrorFile;
    }
  }

  private void remove(final File dest) throws SibincoException {
    if (dest.exists())
        if (dest.delete()!=true) {
          logger.error("Couldn't delete rule " + dest.getAbsolutePath());
          throw new SibincoException("Couldn't delete rule " + dest.getAbsolutePath());
        }
  }

  private void checkType(final String type) throws SibincoException {
    for(int i=0;i<types.length;i++) {
      if (types[i].equals(type)) return;
    }
    logger.error("webapp config - parameter \"type\" in the section \"installation\" doesn't correspond to enumeration: "+Arrays.asList(types));
    throw new SibincoException("webapp config - parameter \"type\" in the section \"installation\" doesn't correspond to enumeration: "+Arrays.asList(types));
  }

  private void checkOperationType(final byte operationType) throws SibincoException {
    for(int i=0;i<operationTypes.length;i++) {
      if(operationTypes[i] == operationType) return;
    }
    logger.error("operationType "+operationType+" doesn't supported by HSDaemon");
    throw new SibincoException("operationType "+operationType+" doesn't supported by HSDaemon");
  }
  
  private class CommonSaver implements SavingStrategy {
      private File source;

      public CommonSaver(File configFile) {
        this.source = configFile;
      }

      public void storeToMirror(final File dest) throws SibincoException {
        OutputStream fos = null;
        InputStream fis = null;
        try
        {
          fos = new FileOutputStream(dest);
          fis = new FileInputStream(source);
          byte[] buf = new byte[32768];
          int read;
          while ((read = fis.read(buf, 0, buf.length)) != -1)
            fos.write(buf, 0, read);
        }
        catch (IOException ioe)
        {
          logger.error( "HSDaemon:CommonSaver:storeToMirror:HS:ERROR:\nCouldn't copy file from path " + source.getAbsolutePath() + " to path " + dest.getAbsolutePath(),ioe );
          logger.error( "HS:ERROR." );
    //      throw new SibincoException("Couldn't copy file from path "+source.getAbsolutePath()+" to path "+dest.getAbsolutePath());
        }
        finally
        {
          try
          {
            if(fos != null)
              fos.close();
            if(fis != null)
              fis.close();
          }
          catch(Exception e)
          {
            e.printStackTrace();
          }
        }
  }

  }
}
