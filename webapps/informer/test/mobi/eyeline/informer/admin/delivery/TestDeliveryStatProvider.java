package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;
import org.junit.Before;

import java.io.File;

/**
 * Интерфейс, обеспечивающий досуп к статистике
 * @author Artem Snopkov
 */
public class TestDeliveryStatProvider extends DeliveryStatProvider{


  public TestDeliveryStatProvider(File directory,FileSystem fileSys) {
        super(directory,fileSys,"'p'yyyyMMdd");
  }

}