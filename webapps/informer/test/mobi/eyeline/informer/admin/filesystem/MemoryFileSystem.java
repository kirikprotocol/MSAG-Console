package mobi.eyeline.informer.admin.filesystem;

import mobi.eyeline.informer.admin.AdminException;

import java.io.*;
import java.util.*;

/**
 * Реализация FileSystem, все данные которой хранятся в памяти.
 *
 * User: artem
 * Date: 25.03.11
 */
public class MemoryFileSystem extends FileSystem {

  private static final byte[] DIRECTORY = new byte[0];

  private Map<String, byte[]> files = new HashMap<String, byte[]>();

  public MemoryFileSystem() {
    files.put(new File("").getAbsolutePath(), DIRECTORY);
  }

  public InputStream getInputStream(File file) throws AdminException {
    if (!files.containsKey(file.getAbsolutePath()))
      throw new FileSystemException("io_error", file.getAbsolutePath() + " does not exists");

    byte[] content = files.get(file.getAbsolutePath());
    return new ByteArrayInputStream(content);
  }

  @Override
  public synchronized OutputStream getOutputStream(final File file, final boolean append) throws AdminException {

    if (!existsParentDir(file))
      throw new FileSystemException("io_error", file.getParent() + " does not exists");

    if (!append)
      files.put(file.getAbsolutePath(), new byte[0]);

    return new ByteArrayOutputStream() {
      public void close() throws IOException {
        super.close();

        byte[] newBytes = toByteArray();

        if (append && files.containsKey(file.getAbsolutePath())) {
          byte[] currentBytes = files.get(file.getAbsolutePath());
          byte[] finalBytes = new byte[newBytes.length + currentBytes.length];
          System.arraycopy(currentBytes, 0, finalBytes, 0, currentBytes.length);
          System.arraycopy(newBytes, 0, finalBytes, currentBytes.length, newBytes.length);
          files.put(file.getAbsolutePath(), finalBytes);
        } else {
          files.put(file.getAbsolutePath(), newBytes);
        }
      }
    };
  }

  private boolean existsParentDir(File file) {
    if (file.getParent() == null)
      return true;

    byte[] parent = files.get(file.getParentFile().getAbsolutePath());
    return parent == DIRECTORY;
  }

  @Override
  public synchronized void rename(File file, File toFile) throws AdminException {
    if (!files.containsKey(file.getAbsolutePath())) {
      String err = "Can't rename file '" + file.getAbsolutePath() + "' to '" + toFile.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error", err);
    }

    if (!existsParentDir(toFile)) {
      String err = "Can't rename file '" + file.getAbsolutePath() + "' to '" + toFile.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error", err);
    }

    byte[] fileContent = files.remove(file.getAbsolutePath());
    files.put(toFile.getAbsolutePath(), fileContent);
  }

  @Override
  public void copy(File file, File toFile) throws AdminException {
    if (!files.containsKey(file.getAbsolutePath())) {
      String err = "Can't copy file '" + file.getAbsolutePath() + "' to '" + toFile.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error", err);
    }

    if (!existsParentDir(toFile)) {
      String err = "Can't copy file '" + file.getAbsolutePath() + "' to '" + toFile.getAbsolutePath() + '\'';
      throw new FileSystemException("io_error", err);
    }

    byte[] bytes = files.get(file.getAbsolutePath());
    files.put(toFile.getAbsolutePath(), bytes);
  }

  @Override
  public synchronized void delete(File file) throws AdminException {
    files.remove(file.getAbsolutePath());
  }

  @Override
  public synchronized void mkdirs(File file) throws AdminException {
    File parent = file.getParentFile();
    while(parent != null) {
      files.put(parent.getAbsolutePath(), DIRECTORY);
      parent = parent.getParentFile();
    }
    files.put(file.getAbsolutePath(), DIRECTORY);
  }

  @Override
  public synchronized boolean exists(File file) throws AdminException {
    return files.containsKey(file.getAbsolutePath());
  }

  @Override
  public synchronized long length(File f) throws AdminException {
    byte[] bytes = files.get(f.getAbsolutePath());
    if (bytes == null)
      return 0;
    if (bytes == DIRECTORY)
      throw new FileSystemException("io_error", "Can't get length of directory!");
    return bytes.length;
  }

  @Override
  public synchronized void createNewFile(File f) throws AdminException {
    if (files.containsKey(f.getAbsolutePath()))
      throw new FileSystemException("io_error", "File already exists: " + f.getAbsolutePath());

    if (!existsParentDir(f))
      throw new FileSystemException("io_error", "Parent directory does not exists: " + f.getParent());

    files.put(f.getAbsolutePath(), new byte[0]);
  }

  public void createNewFile(File file, InputStream is) throws AdminException {
    createNewFile(file);

    ByteArrayOutputStream os = new ByteArrayOutputStream();
    try {
      int b;
      while((b = is.read()) != -1)
        os.write(b);
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      try {
        is.close();
      } catch (IOException ignored) {
      }
    }

    files.put(file.getAbsolutePath(), os.toByteArray());
  }

  public File createNewFile(String file, InputStream is) throws AdminException {
    File f = new File(file);
    createNewFile(f, is);
    return f;
  }

  public File mkdirs(String dir) throws AdminException {
    File f = new File(dir);
    mkdirs(f);
    return f;
  }

  public String[] list(File file) throws AdminException {
    byte[] bytes = files.get(file.getAbsolutePath());
    if (bytes == null || bytes != DIRECTORY)
      throw new FileSystemException("io_error", file.getAbsolutePath() + " is not a directory");

    ArrayList<String> result = new ArrayList<String>();
    for (Map.Entry<String, byte[]> e : files.entrySet()) {
      if (e.getKey().startsWith(file.getAbsolutePath()) && !e.getKey().equals(file.getAbsolutePath()))
        result.add(e.getKey());
    }
    String[] res = new String[result.size()];
    return result.toArray(res);
  }

}
