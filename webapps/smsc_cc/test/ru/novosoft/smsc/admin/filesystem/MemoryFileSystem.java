package ru.novosoft.smsc.admin.filesystem;

import ru.novosoft.smsc.admin.AdminException;

import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * User: artem
 * Date: 28.10.11
 */
public class MemoryFileSystem extends FileSystem {

  private static final FileInfo DIRECTORY = new FileInfo();

  private Map<String, FileInfo> files = new HashMap<String, FileInfo>();

  public MemoryFileSystem() {
    files.put(new File("").getAbsolutePath(), DIRECTORY);
  }

  public MemoryFileSystem(MemoryFileSystem copy) {
    this.files.putAll(copy.files);
  }

  public void clear() {
    files.clear();
  }

  public InputStream getInputStream(File file) throws AdminException {
    if (!files.containsKey(file.getAbsolutePath()))
      throw new FileSystemException("io_error", file.getAbsolutePath() + " does not exists");

    byte[] content = files.get(file.getAbsolutePath()).getContent();
    return new ByteArrayInputStream(content);
  }

  @Override
  public OutputStream getOutputStream(File file) throws AdminException {
    return getOutputStream(file, false);
  }


  public synchronized OutputStream getOutputStream(final File file, final boolean append) throws AdminException {

    if (!existsParentDir(file))
      throw new FileSystemException("io_error", file.getParent() + " does not exists");

    if (!append)
      files.put(file.getAbsolutePath(), new FileInfo(new byte[0]));

    return new ByteArrayOutputStream() {
      public void close() throws IOException {
        super.close();

        byte[] newBytes = toByteArray();

        if (append && files.containsKey(file.getAbsolutePath())) {
          byte[] currentBytes = files.get(file.getAbsolutePath()).getContent();
          byte[] finalBytes = new byte[newBytes.length + currentBytes.length];
          System.arraycopy(currentBytes, 0, finalBytes, 0, currentBytes.length);
          System.arraycopy(newBytes, 0, finalBytes, currentBytes.length, newBytes.length);
          files.put(file.getAbsolutePath(), new FileInfo(finalBytes));
        } else {
          files.put(file.getAbsolutePath(), new FileInfo(newBytes));
        }
      }
    };
  }

  private boolean existsParentDir(File file) {
    if (file.getParent() == null)
      return true;

    FileInfo parent = files.get(file.getParentFile().getAbsolutePath());
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

    byte[] fileContent = files.remove(file.getAbsolutePath()).getContent();
    files.put(toFile.getAbsolutePath(), new FileInfo(fileContent));
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

    byte[] bytes = files.get(file.getAbsolutePath()).getContent();
    files.put(toFile.getAbsolutePath(), new FileInfo(bytes));
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


  public synchronized boolean exists(File file) throws AdminException {
    return files.containsKey(file.getAbsolutePath());
  }

  public synchronized long length(File f) throws AdminException {
    FileInfo bytes = files.get(f.getAbsolutePath());
    if (bytes == null)
      return 0;
    if (bytes == DIRECTORY)
      throw new FileSystemException("io_error", "Can't get length of directory!");
    return bytes.getContent().length;
  }

  public synchronized void createNewFile(File f) throws AdminException {
    if (files.containsKey(f.getAbsolutePath()))
      throw new FileSystemException("io_error", "File already exists: " + f.getAbsolutePath());

    if (!existsParentDir(f))
      throw new FileSystemException("io_error", "Parent directory does not exists: " + f.getParent());

    files.put(f.getAbsolutePath(), new FileInfo(new byte[0]));
  }

  public File createNewFile(File file, InputStream is) throws AdminException {
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

    files.put(file.getAbsolutePath(), new FileInfo(os.toByteArray()));
    return file;
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
    FileInfo bytes = files.get(file.getAbsolutePath());
    if (bytes == null || bytes != DIRECTORY)
      throw new FileSystemException("io_error", file.getAbsolutePath() + " is not a directory");

    ArrayList<String> result = new ArrayList<String>();
    for (Map.Entry<String, FileInfo> e : files.entrySet()) {
      if (e.getKey().startsWith(file.getAbsolutePath()) && !e.getKey().equals(file.getAbsolutePath())) {
        result.add(e.getKey().substring(e.getKey().lastIndexOf(file.getAbsolutePath())+file.getAbsolutePath().length()+1));
      }
    }
    String[] res = new String[result.size()];
    return result.toArray(res);
  }

  public File[] listFiles(File file) {
    FileInfo bytes = files.get(file.getAbsolutePath());
    if (bytes == null || bytes != DIRECTORY)
      throw new IllegalArgumentException(file.getAbsolutePath() + " is not a directory");

    ArrayList<String> result = new ArrayList<String>();
    for (Map.Entry<String, FileInfo> e : files.entrySet()) {
      if (e.getKey().startsWith(file.getAbsolutePath()) && !e.getKey().equals(file.getAbsolutePath()))
        result.add(e.getKey());
    }
    File[] res = new File[result.size()];
    int i = 0;
    for(String s : result) {
      res[i] = new File(s);
      i++;
    }
    return res;
  }

  public File[] listFiles(File file, FileFilter fileFilter) {
    FileInfo bytes = files.get(file.getAbsolutePath());
    if (bytes == null || bytes != DIRECTORY)
      throw new IllegalArgumentException(file.getAbsolutePath() + " is not a directory");

    ArrayList<String> result = new ArrayList<String>();
    for (Map.Entry<String, FileInfo> e : files.entrySet()) {
      if (e.getKey().startsWith(file.getAbsolutePath()) && !e.getKey().equals(file.getAbsolutePath()) && fileFilter.accept(new File(e.getKey())))
        result.add(e.getKey());
    }
    File[] res = new File[result.size()];
    int i = 0;
    for(String s : result) {
      res[i] = new File(s);
      i++;
    }
    return res;
  }

  @Override
  public long lastModified(File file) {
    FileInfo f = files.get(file.getAbsolutePath());
    if (f == null)
      return 0L;
    return f.getLastModified();
  }

  @Override
  public boolean isDirectory(File file) {
    FileInfo f = files.get(file.getAbsolutePath());
    return f != null && f == DIRECTORY;
  }

  private static class FileInfo {
    private long lastModified;
    private byte[] content;

    private FileInfo() {
    }

    private FileInfo(byte[] content) {
      this.lastModified = System.currentTimeMillis();
      this.content = content;
    }

    public long getLastModified() {
      return lastModified;
    }

    public void setLastModified(long lastModified) {
      this.lastModified = lastModified;
    }

    public byte[] getContent() {
      return content;
    }

    public void setContent(byte[] content) {
      this.content = content;
    }
  }
}
