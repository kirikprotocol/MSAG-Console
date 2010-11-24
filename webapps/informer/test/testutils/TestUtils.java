package testutils;

import junit.framework.AssertionFailedError;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.*;

import java.io.*;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;

/**
 * @author Artem Snopkov
 */
public class TestUtils {

  public static File createRandomFile(String ext) {
    File res;
    do {
      res = new File(System.currentTimeMillis() + ext);
    } while (res.exists());

    return res;
  }

  public static File createRandomDir(String syffix) {
    File dir = createRandomFile(syffix);
    if (dir.mkdirs())
      return dir;
    return null;
  }

  public static void exportResource(InputStream is, File toFile) throws IOException {
    if(toFile.getParentFile() != null && !toFile.getParentFile().exists() && !toFile.getParentFile().mkdirs()) {
      throw new IOException("Can't create file: "+toFile.getParent());
    }
    OutputStream os = null;
    try {
      os = new BufferedOutputStream(new FileOutputStream(toFile));

      int b;
      while ((b = is.read()) >= 0)
        os.write(b);
    } catch (EOFException e) {
    } catch (Exception e) {
      e.printStackTrace();
      throw new AssertionFailedError(e.getMessage());
    } finally {
      if (is != null)
        is.close();
      if (os != null)
        os.close();
    }
  }

  public static void exportResource(InputStream is, File toFile, boolean override) throws IOException {
    if (!override && toFile.exists())
      return;
    exportResource(is, toFile);
  }

  public static File exportResourceToRandomFile(InputStream is, String fileExt) throws IOException {
    File file = createRandomFile(fileExt);
    exportResource(is, file);
    return file;
  }

  public static boolean recursiveDeleteFolder(File folder) {
    File[] childs = folder.listFiles();
    if (childs != null) {
      for (File child : childs) {
        if (child.isDirectory()) {
          if (!recursiveDeleteFolder(child))
            return false;
        } else {
          if (!child.delete())
            return false;
        }
      }
    }
    return folder.delete();
  }


  public static void copyDirectory(File sourceLocation, File targetLocation, FileSystem fileSystem) throws IOException, AdminException {

    if (sourceLocation.isDirectory()) {
      if (!fileSystem.exists(targetLocation)) {
        fileSystem.mkdirs(targetLocation);
      }
      String[] children = fileSystem.list(sourceLocation);
      for (int i=0; i<children.length; i++) {
        copyDirectory(new File(sourceLocation, children[i]),
            new File(targetLocation, children[i]), fileSystem);
      }
    }
    else {
      fileSystem.copy(sourceLocation,targetLocation);
    }
  }

  public static void extractDirFromJar(InputStream is, String jarInternalPathURI, File dstStatDir, FileSystem fileSys) throws AdminException, IOException, URISyntaxException {

    JarInputStream jis = new JarInputStream(is);
    JarEntry je;
    while (( je = jis.getNextJarEntry())!=null) {
      String name = je.getName();
      if(!je.isDirectory() && (name+File.separatorChar).startsWith(jarInternalPathURI+File.separatorChar)) {
        String relPath = name.substring(jarInternalPathURI.length());
        File dstFile = new File(dstStatDir,relPath.replace('/',File.separatorChar));
        extractFile(jis,dstFile,fileSys);
      }

    }

  }

  private static void extractFile(JarInputStream jis, File dstFile, FileSystem fileSys) throws IOException, AdminException {
      String parentDir = dstFile.getParent();
      if(parentDir!=null) {
        File d = new File(parentDir);
        if(!fileSys.exists(d)) fileSys.mkdirs(d);
      }
      OutputStream os = null;
      try {
        os = fileSys.getOutputStream(dstFile,false);
        byte[] b = new byte[1024];
        int len;
        while ((len = jis.read(b, 0, b.length)) != -1) {
          os.write(b, 0, len);
        }
        jis.closeEntry();
        os.close();
      }
      finally {
        os.close();
      }
  }
}
