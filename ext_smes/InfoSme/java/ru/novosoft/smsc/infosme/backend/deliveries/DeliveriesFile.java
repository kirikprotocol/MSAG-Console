package ru.novosoft.smsc.infosme.backend.deliveries;

import java.io.*;
import java.util.ArrayList;
import java.util.Collections;

/**
 * User: artem
* Date: 29.01.2009
*/
public class DeliveriesFile {

  private int region;
  private boolean containsTexts;
  private final File file;
  private OutputStream os;
  private final ArrayList buffer;

  private int totalSize;

  DeliveriesFile(int region, File file) {
    this.region = region;
    this.file = file;
    this.os = null;
    this.buffer = new ArrayList(1000);
  }

  public boolean isContainsTexts() {
    return containsTexts;
  }

  public void setContainsTexts(boolean containsTexts) {
    this.containsTexts = containsTexts;
  }

  public void addLine(String msisdn) throws IOException {
    buffer.add(msisdn);
    totalSize++;
    if (buffer.size()>=1000)
      flushBuffer();
  }

  private void flushBuffer() throws IOException {
    if (os == null) {
      os = new BufferedOutputStream(new FileOutputStream(file));
      file.deleteOnExit();
    }

    Collections.shuffle(buffer);

    for (int i=0; i < buffer.size(); i++) {
      os.write(((String)buffer.get(i)).getBytes());
      os.write('\n');
    }

    buffer.clear();
  }

  public void close() throws IOException {
    flushBuffer();
    os.close();
  }

  public int getTotalSize() {
    return totalSize;
  }

  public File getFile() {
    return file;
  }

  public int getRegion() {
    return region;
  }
}
