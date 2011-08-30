package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.junit.Test;

import java.io.File;

import static org.junit.Assert.*;

/**
 * User: artem
 * Date: 11.07.11
 */
public class DetailedSaveStrategySyncTest {

  private File workDir=new File("work");
  private File localCopyDir = new File(workDir, "detailedLocalCopy");

  private DetailedSaveStrategy createStrategy(MemoryFileSystem fs, FileResourceStub remoteResource) throws AdminException {
    UserCPsettings s = new UserCPsettings();
    s.setCreateReports(true);
    ResourceOptions opts =  new ResourceOptions(new User(), workDir, s);
    ContentProviderContextStub ctx = new ContentProviderContextStub(fs);
    return new DetailedSaveStrategy(ctx, remoteResource, opts);
  }

  private void testSync(String beforeInLocal, String beforeOnResource, String afterInLocal, String afterOnResource) throws AdminException {
    MemoryFileSystem fs = new MemoryFileSystem();
    FileResourceStub remoteResource = new FileResourceStub();

    fs.mkdirs(localCopyDir);

    if (beforeInLocal != null)
      fs.createNewFile(new File(localCopyDir, beforeInLocal));

    if (beforeOnResource != null)
      remoteResource.addFile(beforeOnResource);

    DetailedSaveStrategy strategy = createStrategy(fs, remoteResource);

    strategy.synchronize(true);

    if (afterInLocal != null) {
      assertTrue(fs.exists(new File(localCopyDir, afterInLocal)));
      assertEquals(1, fs.list(localCopyDir).length);
    }

    if (afterOnResource != null) {
      remoteResource.open();
      assertTrue(contains(remoteResource, afterOnResource));
      if (beforeOnResource != null && !beforeOnResource.equals(afterOnResource))
        assertFalse(contains(remoteResource, beforeOnResource));
      remoteResource.close();
    }
  }

  private boolean contains(FileResource resource, String file) throws AdminException {
    return resource.listFiles().contains(file);
  }

  @Test
  public void testSync() throws AdminException {
    testSync(null, "1.csv1", null, "1.csv1");
    testSync(null, "1.notcsv", null, "1.notcsv");
    testSync(null, "1.csv", "1.csv", "1.csv");

    testSync("1.csv", "1.csv", "1.csv", "1.csv");
    testSync("1.csv", "1.csv.active", "1.csv", "1.csv.active");
    testSync("1.csv", "1.csv.finished", "1.csv", "1.csv.finished");

    testSync("1.csv.active", "1.csv", "1.csv.active", "1.csv.active");
    testSync("1.csv.active", "1.csv.active", "1.csv.active", "1.csv.active");
    testSync("1.csv.active", "1.csv.finished", null, "1.csv.finished");

    testSync("1.csv.finished", "1.csv", "1.csv", "1.csv");
    testSync("1.csv.finished", "1.csv.active", "1.csv.finished", "1.csv.finished");
    testSync("1.csv.finished", "1.csv.finished", "1.csv.finished", "1.csv.finished");
  }

  @Test
  public void testReportUpload() throws AdminException {
    MemoryFileSystem fs = new MemoryFileSystem();
    FileResourceStub remoteResource = new FileResourceStub();

    fs.mkdirs(localCopyDir);
    fs.createNewFile(new File(localCopyDir, "1.csv.finished"));
    fs.createNewFile(new File(localCopyDir, "1.csv.report"));

    remoteResource.addFile("1.csv.active");

    DetailedSaveStrategy strategy = createStrategy(fs, remoteResource);
    strategy.synchronize(true);

    remoteResource.open();
    assertTrue(contains(remoteResource, "1.csv.finished"));
    assertTrue(contains(remoteResource, "1.csv.report"));
    assertFalse(contains(remoteResource, "1.csv.active"));
    remoteResource.close();
  }

}
