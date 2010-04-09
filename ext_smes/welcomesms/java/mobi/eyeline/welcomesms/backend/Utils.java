package mobi.eyeline.welcomesms.backend;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.nio.channels.FileChannel;

/**
 * author: alkhal
 */
public class Utils {

  public static void truncateFile(RandomAccessFile f, long index) throws IOException {
    FileChannel fc = null;
    try {
      fc = f.getChannel();
      fc.truncate(index);
    } finally {
      if (fc != null)
        fc.close();
    }
  }
  
}
