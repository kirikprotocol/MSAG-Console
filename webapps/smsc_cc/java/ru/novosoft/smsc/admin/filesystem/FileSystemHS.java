package ru.novosoft.smsc.admin.filesystem;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;

import java.io.*;

/**
 * @author Aleksandr Khalitov
 */
class FileSystemHS extends FileSystem {

  private File baseDir;

  private File[] mirrorsDir;

  private boolean error;

  private String errStr;

  FileSystemHS(File baseDir, File... mirrorsDir) {
    assert baseDir != null && mirrorsDir != null : "Some arguments are null";
    this.baseDir = baseDir;
    this.mirrorsDir = mirrorsDir;
  }

  @Override
  public OutputStream getOutputStream(File file) throws AdminException {
    assert file != null : "Some arguments are null";
    checkErrors();
    File[] mirrors = getMirrorsFiles(file);
    try {
      final FileOutputStream[] oss = new FileOutputStream[mirrorsDir.length];
      int i = 0;
      for (File f : mirrors) {
        oss[i] = new FileOutputStream(f);
        i++;
      }
      return new HSFileOutputStream(file, oss);
    } catch (IOException e) {
      errStr = e.getMessage();
      error = true;
      logger.error(e, e);
      throw new AdminException(e.getMessage());
    }
  }

  @Override
  public void rename(File file, File toFile) throws AdminException {
    assert file != null && toFile != null : "Some arguments are null";
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
        logger.error(errStr);
        throw new AdminException(errStr);
      }
    }
  }

  @Override
  public void copy(File file, File toFile) throws AdminException {
    assert file != null && toFile != null : "Some arguments are null";
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    File[] toFiles = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    getMirrorsFiles(toFile, toFiles);
    files[mirrorsDir.length] = file;
    toFiles[mirrorsDir.length] = toFile;


    for (int i = files.length - 1; i > -1; i--) {
      try {
        Functions.copyFileTo(files[i], toFiles[i]);
      } catch (IOException e) {
        error = true;
        errStr = "Can't copy file '" + files[i].getAbsolutePath() + "' to '" + toFiles[i].getAbsolutePath() + '\'';
        logger.error(e, e);
        throw new AdminException(errStr);
      }
    }

  }

  @Override
  public void delete(File file) throws AdminException {
    assert file != null : "Some arguments are null";
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    files[mirrorsDir.length] = file;

    for (int i = files.length - 1; i > -1; i--) {
      if (!files[i].delete() && files[i].exists()) {
        error = true;
        errStr = "Can't remove file '" + files[i].getAbsolutePath() + '\'';
        logger.error(errStr);
        throw new AdminException(errStr);
      }
    }
  }

  @Override
  public void mkdirs(File file) throws AdminException {
    assert file != null : "Some arguments are null";
    checkErrors();

    File[] files = new File[mirrorsDir.length + 1];
    getMirrorsFiles(file, files);
    files[mirrorsDir.length] = file;

    for (int i = files.length - 1; i > -1; i--) {
      if (!files[i].mkdirs() && !files[i].exists()) {
        error = true;
        errStr = "Can't mkdirs for file '" + files[i].getAbsolutePath() + '\'';
        logger.error(errStr);
        throw new AdminException(errStr);
      }
    }
  }

  @Override
  public boolean exists(File file) throws AdminException {
    assert file != null : "Some arguments are null";
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
      throw new AdminException("File '" + absolutePath + "' must be in subdirectories of '" + baseDir + '\'');
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

//  public static void main(String[] args) throws Exception{
//    FileSystemHS fs = new FileSystemHS(new File(""), new File("mirror"));
//    PrintWriter pw = null;
//    try{
//      pw = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(new File("test.test")))));
//      pw.println("It's working!!!!!");
//    }finally {
//      if(pw != null) {
//        pw.close();
//      }
//    }
//  }

  private void checkErrors() throws AdminException {
    if (error) {
      throw new AdminException("There was a error at some time ago. Further work is impossible. ERROR: '" + errStr + '\'');
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
