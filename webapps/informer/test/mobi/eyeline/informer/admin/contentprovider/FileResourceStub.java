package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;

import java.io.*;
import java.util.*;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * User: artem
 * Date: 08.07.11
 */
class FileResourceStub extends FileResource  {

  private MemoryFileSystem fs = new MemoryFileSystem();
  private boolean opened;
  private File workDir = new File("resource");

  private Set<String> filesDeprecatedToRemove = new HashSet<String>();
  private Set<String> filesDeprecatedToLoad = new HashSet<String>();

  public void addFile(String fileName, String... lines) throws AdminException {
    fs.mkdirs(workDir);
    StringBuilder sb = new StringBuilder();
    for (String line : lines)
      sb.append(line).append('\n');

    ByteArrayInputStream is = new ByteArrayInputStream(sb.toString().getBytes());
    try {
      fs.createNewFile(new File(workDir, fileName), is);
    } catch (AdminException e) {
      e.printStackTrace();
    }
  }

  public void deprecateRemoveFile(String path) {
    filesDeprecatedToRemove.add(path);
  }

  public void allowRemoveFile(String path) {
    filesDeprecatedToRemove.remove(path);
  }

  public void deprecateLoadFile(String path) {
    filesDeprecatedToLoad.add(path);
  }

  private void checkResourceIsOpened() {
    assertTrue("File resource should be opened before use", opened);
  }



  @Override
  public void open() throws AdminException {
    opened = true;
    fs.mkdirs(workDir);
  }

  @Override
  public List<String> listFiles() throws AdminException {
    checkResourceIsOpened();
    String[] files =  fs.list(workDir);
    List<String> res = new ArrayList<String>(files.length);
    Collections.addAll(res, files);
    return res;
  }

  @Override
  public void get(String path, OutputStream os) throws AdminException {
    checkResourceIsOpened();

    if (filesDeprecatedToLoad.contains(path)) {
      throw new ContentProviderException("Load error");
    }

    InputStream is = null;
    try {
      is = fs.getInputStream(new File(workDir, path));
      int b;
      while((b = is.read()) != -1)
        os.write(b);
    } catch (IOException e) {
      fail(e.getMessage());
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {}
    }
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    checkResourceIsOpened();
    fs.rename(new File(workDir, fromPath), new File(workDir, toPath));
  }

  @Override
  public void remove(String path) throws AdminException {
    checkResourceIsOpened();
    if (filesDeprecatedToRemove.contains(path)) {
      throw new ContentProviderException("Remove error");
    }
    fs.delete(new File(workDir, path));
  }

  @Override
  public void put(InputStream is, String toPath) throws AdminException {
    checkResourceIsOpened();
    fs.createNewFile(new File(workDir, toPath), is);
  }

  @Override
  public void close() throws AdminException {
    checkResourceIsOpened();
    opened = false;
  }
}
