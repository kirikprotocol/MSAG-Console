package ru.novosoft.smsc.util;

import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class IOUtilsTest {

  @Test
  public void testWriteUInt8() throws Exception {
    int[] values= new int[]{10,20,30,255};

    for (int val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(10);
      IOUtils.writeUInt8(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readUInt8(is));
    }
  }

  @Test
  public void testWriteUInt16() throws Exception {
    int[] values= new int[]{10,20,30,255, 10000, 65000};

    for (int val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(10);
      IOUtils.writeUInt16(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readUInt16(is));
    }
  }

  @Test
  public void testWriteUInt32() throws Exception {
    int[] values= new int[]{10,20,30,255, 10000, 1000000};

    for (int val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(10);
      IOUtils.writeUInt32(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readUInt32(is));
    }
  }

  @Test
  public void testWriteInt64() throws Exception {
    int[] values= new int[]{10,20,30,255, 10000, -1000000};

    for (int val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(10);
      IOUtils.writeInt64(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readInt64(is));
    }
  }

  @Test
  public void testWriteString8() throws Exception {
    String[] values= new String[]{"Привет мир", "Hello world"};

    for (String val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(500);
      IOUtils.writeString8(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readString8(is));
    }
  }

  @Test
  public void testWriteString16() throws Exception {
    String[] values= new String[]{"Привет мир", "Hello world"};

    for (String val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(500);
      IOUtils.writeString16(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readString16(is));
    }
  }

  @Test
  public void testWriteString32() throws Exception {
    String[] values= new String[]{"Привет мир", "Hello world"};

    for (String val : values) {
      ByteArrayOutputStream os = new ByteArrayOutputStream(500);
      IOUtils.writeString32(os, val);
      ByteArrayInputStream is = new ByteArrayInputStream(os.toByteArray());
      assertEquals(val, IOUtils.readString32(is));
    }
  }
}
