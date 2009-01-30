package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import com.eyeline.utils.IOUtils;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.EOFException;

/**
 * User: artem
 * Date: 06.08.2008
 */

class RandomAccessFileReader {
  private final RandomAccessFile f;

  private final byte[] buffer;
  private long bufferStartPos;
  private int bufferPos;
  private int bufferSize;

  public RandomAccessFileReader(RandomAccessFile f) {
    this(f, 8192);
  }

  public RandomAccessFileReader(RandomAccessFile f, int bufferSize) {
    this.f = f;
    this.buffer = new byte[bufferSize];
  }

  public void seek(long pos) throws IOException {
    if (pos >= bufferStartPos && pos < bufferStartPos + bufferSize)
      bufferPos = (int)(pos - bufferStartPos);
    else {
      bufferSize = 0;
      bufferStartPos = pos;
      bufferPos = 0;
    }
  }

  public void clean() {
    bufferStartPos = 0;
    bufferPos = 0;
    bufferSize = 0;
  }

  public long getFilePointer() {
    return bufferStartPos + bufferPos;
  }

  public int readInt() throws IOException {
    byte[] bytes = new byte[4];
    readFully(bytes);
    return IOUtils.readInt(bytes, 0);
  }

  public int readFully(byte[] bytes) throws IOException {
    int totalLen = 0;
    do {
      if (bufferPos >= bufferSize)
        fillBuffer();

      final int len = Math.min(bufferSize - bufferPos, bytes.length - totalLen);
      if (len > 0) {
        System.arraycopy(buffer, bufferPos, bytes, totalLen, len);
        totalLen+=len;
        bufferPos += len;
      }

    } while(totalLen < bytes.length);

    return totalLen;
  }

  public void fillBuffer() throws IOException {
    bufferStartPos += bufferSize;

    long len = f.length();
    if (bufferStartPos >= len)
      throw new EOFException();

    if (bufferStartPos > len - buffer.length) {
      long nStartPos = (len - buffer.length);
      if (nStartPos < 0)
        nStartPos = 0;
      bufferPos = (int)(bufferStartPos - nStartPos);
      bufferStartPos = nStartPos;
    } else {
      bufferPos = 0;
    }

    int actualSize = (int)Math.min(len, buffer.length);
    f.seek(bufferStartPos);
    f.readFully(buffer, 0, actualSize);

    bufferSize = actualSize;
  }
}
