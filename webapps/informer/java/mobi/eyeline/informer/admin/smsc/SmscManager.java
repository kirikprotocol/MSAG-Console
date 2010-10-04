package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;

import java.io.File;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class SmscManager {


  private Infosme infosme;

  private FileSystem fileSystem;

  private File config;


  public SmscManager(Infosme infosme, File config, FileSystem fileSystem) {
    this.infosme = infosme;
    this.fileSystem = fileSystem;
    this.config = config;
  }






  public void addSmsc(Smsc smscName) throws AdminException{

  }

  public void getSmsc(String smscName) throws AdminException{

  }

  public List<String> getSmscs() throws AdminException{
    
    return null;
  }

  public void removeSmsc(String smscName) throws AdminException{

  }



}
