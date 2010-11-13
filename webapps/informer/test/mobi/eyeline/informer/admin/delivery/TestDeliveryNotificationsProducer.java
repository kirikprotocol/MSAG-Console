package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 11:41:12
 */
public class TestDeliveryNotificationsProducer extends DeliveryNotificationsProducer{
  public TestDeliveryNotificationsProducer(File directory, FileSystem fileSys) throws InitException {
    super(directory, fileSys);
  }
}
