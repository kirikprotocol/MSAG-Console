package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatProvider;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * Интерфейс, обеспечивающий досуп к статистике
 * @author Artem Snopkov
 */
public class TestDeliveryStatProvider extends DeliveryStatProvider {


  public TestDeliveryStatProvider(File directory,FileSystem fileSys) {
        super(directory,fileSys,"'p'yyyyMMdd");
  }

}