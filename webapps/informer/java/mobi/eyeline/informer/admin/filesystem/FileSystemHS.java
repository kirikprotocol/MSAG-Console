package mobi.eyeline.informer.admin.filesystem;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.FileUtils;

import java.io.*;

/**
 * Файловая система для HS-установки
 *
 * @author Aleksandr Khalitov
 */
class FileSystemHS extends FileSystem {

  private File baseDir;

  private File[] mirrorsDir;

  private boolean error;

  private String errStr;

  FileSystemHS(File baseDir, File... mirrorsDir) {
    if (baseDir == null || mirrorsDir == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    this.baseDir = baseDir;
    this.mirrorsDir = mirrorsDir;
  }

  @Override
  public OutputStream getOutputStream(File file, boolean append) throws AdminException {
    if (file == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    checkErrors();
    File[] mirrors = getMirrorsFiles(file);
    try {
      final FileOutputStream[] oss = new FileOutputStream[mirrorsDir.length];
      int i = 0;
      for (File f : mirrors) {
        oss[i] = new FileOutputStream(f, append);
        i++;
      }
      return new HSFileOutputStream(file, oss);
    } catch (IOException e) {
      errStr = e.getMessage();
      error = true;
      throw new FileSystemException("io_error", e);
    }
  }

  @Override
  public void rename(File file, File toFile) throws AdminException {
    if (file == null || toFile == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    File[] toFiles = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    getMirrorsFiles(toFile, toFiles);
    files[mirrorsDir.length] = file;
    toFiles[mirrorsDir.length] = toFile;


    for (int i = files.length - 1; i > -1; i--) {
      if (!files[i].renameTo(toFiles[i])) {
        error = true;
        errStr = "Can't rename file '" + files[i].getAbsolutePath() + "' to '" + toFiles[i].getAbsolutePath() + '\'';
        throw new FileSystemException("io_error", errStr);
      }
    }
  }

  @Override
  public void copy(File file, File toFile) throws AdminException {
    if (file == null || toFile == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    File[] toFiles = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    getMirrorsFiles(toFile, toFiles);
    files[mirrorsDir.length] = file;
    toFiles[mirrorsDir.length] = toFile;


    for (int i = files.length - 1; i > -1; i--) {
      try {
        FileUtils.copyFileTo(files[i], toFiles[i]);
      } catch (IOException e) {
        error = true;
        errStr = "Can't copy file '" + files[i].getAbsolutePath() + "' to '" + toFiles[i].getAbsolutePath() + '\'';
        throw new FileSystemException("io_error", errStr);
      }
    }

  }

  @Override
  public void delete(File file) throws AdminException {
    if (file == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    files[mirrorsDir.length] = file;

    for (int i = files.length - 1; i > -1; i--) {
      if (!files[i].delete() && files[i].exists()) {
        error = true;
        errStr = "Can't remove file '" + files[i].getAbsolutePath() + '\'';
        throw new FileSystemException("io_error", errStr);
      }
    }
  }

  @Override
  public void mkdirs(File file) throws AdminException {
    if (file == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    files[mirrorsDir.length] = file;

    for (int i = files.length - 1; i > -1; i--) {
      if (!files[i].mkdirs() && !files[i].exists()) {
        error = true;
        errStr = "Can't mkdirs for file '" + files[i].getAbsolutePath() + '\'';
        throw new FileSystemException("io_error", errStr);
      }
    }
  }

  @Override
  public boolean exists(File file) throws AdminException {
    if (file == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    files[mirrorsDir.length] = file;

    for (int i = files.length - 1; i > -1; i--) {
      if (!files[i].exists()) {
        return false;
      }
    }
    return true;
  }

  private File[] getMirrorsFiles(File baseFile) throws AdminException {
    File[] files = new File[mirrorsDir.length];
    getMirrorsFiles(baseFile, files);
    return files;
  }

  private void getMirrorsFiles(File baseFile, File[] result) throws AdminException {
    assert baseFile != null : "Some arguments are null";
    String absolutePath = baseFile.getAbsolutePath();
    if (!absolutePath.startsWith(baseDir.getAbsolutePath())) {
      throw new FileSystemException("mirror_error", "File '" + absolutePath + "' must be in subdirectories of '" + baseDir + '\'');
    }
    String relativePath = absolutePath.substring(baseDir.getAbsolutePath().length());
    if (relativePath.startsWith(File.separator)) {
      relativePath = relativePath.substring(File.separator.length());
    }
    int i = 0;
    for (File md : mirrorsDir) {
      String path = md.getAbsolutePath();
      if (!path.endsWith(File.separator)) {
        path = path + File.separator;
      }
      result[i] = new File(path + relativePath);
      i++;
    }
  }

  private void checkErrors() throws AdminException {
    if (error) {
      throw new FileSystemException("mirror_error", errStr);
    }
  }

  private class HSFileOutputStream extends FileOutputStream {

    private final FileOutputStream[] oss;

    public HSFileOutputStream(File file, FileOutputStream[] oss) throws FileNotFoundException {
      super(file);
      this.oss = oss;
    }

    @Override
    public void write(int b) throws IOException {
      try {
        super.write(b);
        for (FileOutputStream fos : oss) {
          fos.write(b);
        }
      } catch (IOException e) {
        errStr = e.getMessage();
        error = true;
        throw e;
      }
    }

    @Override
    public void close() throws IOException {
      try {
        super.close();
        for (FileOutputStream fos : oss) {
          fos.close();
        }
      } catch (IOException e) {
        errStr = e.getMessage();
        error = true;
        throw e;
      }
    }

    @Override
    public void flush() throws IOException {
      try {
        super.flush();
        for (FileOutputStream fos : oss) {
          fos.flush();
        }
      } catch (IOException e) {
        errStr = e.getMessage();
        error = true;
        throw e;
      }
    }

    @Override
    public void write(byte[] b) throws IOException {
      try {
        super.write(b);
        for (FileOutputStream fos : oss) {
          fos.write(b);
        }
      } catch (IOException e) {
        errStr = e.getMessage();
        error = true;
        throw e;
      }
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
      try {
        super.write(b, off, len);
        for (FileOutputStream fos : oss) {
          fos.write(b, off, len);
        }
      } catch (IOException e) {
        errStr = e.getMessage();
        error = true;
        throw e;
      }
    }
  }
}
