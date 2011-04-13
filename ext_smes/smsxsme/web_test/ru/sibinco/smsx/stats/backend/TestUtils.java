package ru.sibinco.smsx.stats.backend;

import java.io.File;

/**
 * @author Aleksandr Khalitov
 */
public class TestUtils {

  public static File createTestDir(String prefix) {
    File f = new File(new StringBuffer().append(prefix).append('-').append(System.currentTimeMillis()).toString());
    if(!f.mkdirs()) {
      throw new RuntimeException("Can't create dir: "+f.getAbsolutePath());
    }
    return f;
  }

  public static void recursiveDirRemove(File dir) {
    if(dir.isDirectory()) {
      File[] files = dir.listFiles();
      for(int i=0;i<files.length;i++) {
        File f = files[i];
        if(f.isDirectory()) {
          recursiveDirRemove(f);
        }else {
          f.delete();
        }
      }
      dir.delete();
    }
  }
}
