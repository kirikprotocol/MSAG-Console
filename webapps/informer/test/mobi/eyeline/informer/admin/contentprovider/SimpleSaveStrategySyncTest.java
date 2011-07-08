package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.Test;

import java.io.File;
import java.util.Arrays;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

/**
 * User: artem
 * Date: 08.07.11
 */
public class SimpleSaveStrategySyncTest {

  private MemoryFileSystem fs = new MemoryFileSystem();
  private ContentProviderContextStub ctx = new ContentProviderContextStub(fs);
  private FileResourceStub remoteResource = new FileResourceStub();

  private File workDir=new File("work");
  private File localCopyDir = new File(workDir, "simpleLocalCopy");

  private SimpleSaveStrategy createStrategy() throws AdminException {
    ResourceOptions opts =  new ResourceOptions(new User(), workDir, new UserCPsettings());
    return new SimpleSaveStrategy(ctx, remoteResource, opts);
  }

  private void assertFileExistsInLocalCopy(String fileName) throws AdminException {
    assertTrue(fs.exists(localCopyDir));
    boolean exists = false;
    for (String f : fs.list(localCopyDir))
      exists = exists || (f.equals(fileName));
    assertTrue(exists);
  }

  private void assertFileDoesNotExistsInLocalCopy(String fileName) throws AdminException {
    assertTrue(fs.exists(localCopyDir));
    for (String f : fs.list(localCopyDir))
      assertFalse(f.equals(fileName));
  }

  private void assertFileExistsOnResource(String fileName) throws AdminException {
    remoteResource.open();
    assertTrue(remoteResource.contains(fileName));
    remoteResource.close();
  }

  private void assertFileDoesNotExistsOnResource(String fileName) throws AdminException {
    remoteResource.open();
    assertFalse(remoteResource.contains(fileName));
    remoteResource.close();
  }

  @Test
  public void testDownloadOnlyFiles() throws AdminException {
    remoteResource.addFile("test1.csv");
    remoteResource.addFile("test2.csv");
    remoteResource.addFile("test2.notcsv");

    SimpleSaveStrategy s = createStrategy();
    s.synchronize(true);

    assertFileExistsInLocalCopy("test1.csv");
    assertFileDoesNotExistsOnResource("test1.csv");
    assertFileExistsInLocalCopy("test2.csv");
    assertFileDoesNotExistsOnResource("test2.csv");
    assertFileDoesNotExistsInLocalCopy("test2.notcsv");
  }

  @Test
  public void testBehavOnLoadError() throws AdminException {
    remoteResource.addFile("test.csv");
    remoteResource.deprecateLoadFile("test.csv");

    SimpleSaveStrategy s = createStrategy();
    try {
      s.synchronize(true);
      fail("sync should throw exception");
    } catch (AdminException ignored) {}

    assertFileDoesNotExistsInLocalCopy("test.csv");
    assertFileExistsOnResource("test.csv");
  }

  @Test
  public void testBehavOnDeleteFileError() throws AdminException {
    remoteResource.addFile("test.csv",
        "+79139495113|hello");

    remoteResource.deprecateRemoveFile("test.csv");

    SimpleSaveStrategy s = createStrategy();
    try {
      s.synchronize(true);
      fail("sync should throw exception");
    } catch (AdminException expected) {}

    assertFileExistsInLocalCopy("test.csv");
    assertFileExistsOnResource("test.csv");
  }

  @Test
  public void testSyncFilesWithEqualsNames() throws AdminException {
    remoteResource.addFile("test.csv");
    fs.mkdirs(localCopyDir);
    fs.createNewFile(new File(localCopyDir, "test.csv"));

    SimpleSaveStrategy s = createStrategy();
    s.synchronize(false);

    assertFileExistsInLocalCopy("test.csv");
    assertFileDoesNotExistsOnResource("test.csv");
  }

  @Test
  public void testFileExistsOnlyInLocalCopy() throws AdminException {
    fs.mkdirs(localCopyDir);
    fs.createNewFile(new File(localCopyDir, "test.csv"));

    SimpleSaveStrategy s = createStrategy();
    s.synchronize(true);

    assertFileExistsInLocalCopy("test.csv");
    assertFileDoesNotExistsOnResource("test.csv");
  }



}
