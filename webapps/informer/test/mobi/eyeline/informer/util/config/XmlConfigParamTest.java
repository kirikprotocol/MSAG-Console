package mobi.eyeline.informer.util.config;

import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import static org.junit.Assert.*;

/**
 * User: artem
 * Date: 24.03.11
 */
public class XmlConfigParamTest {

  @Test
  public void testCreateEmpty() {
    XmlConfigParam p = new XmlConfigParam("myparam");
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
    assertEquals("myparam", p.getName());
    assertEquals("", p.getValue());
  }

  @Test
  public void testCreateString() {
    XmlConfigParam p = new XmlConfigParam("myname", "myvalue", XmlConfigParam.Type.STRING);
    assertEquals("myname", p.getName());
    assertEquals("myvalue", p.getValue());
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testCreateInt() {
    XmlConfigParam p = new XmlConfigParam("myname", "myvalue", XmlConfigParam.Type.INT);
    assertEquals("myname", p.getName());
    assertEquals("myvalue", p.getValue());
    assertEquals(XmlConfigParam.Type.INT, p.getType());
  }

  @Test
  public void testCreateBool() {
    XmlConfigParam p = new XmlConfigParam("myname", "myvalue", XmlConfigParam.Type.BOOL);
    assertEquals("myname", p.getName());
    assertEquals("myvalue", p.getValue());
    assertEquals(XmlConfigParam.Type.BOOL, p.getType());
  }

  @Test
  public void testSetName() throws Exception {
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setName("newName");
    assertEquals("newName", p.getName());
  }

  @Test
  public void testSetInt() throws Exception {
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setInt(10);
    assertEquals(10, p.getInt());
    assertEquals(XmlConfigParam.Type.INT, p.getType());
  }

  @Test
  public void testSetLong() throws Exception {
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setLong(10);
    assertEquals(10, p.getLong());
    assertEquals(XmlConfigParam.Type.INT, p.getType());
  }

  @Test
  public void testSetDate() throws Exception {
    XmlConfigParam p = new XmlConfigParam("myparam");
    Date d = new Date();
    String dateFormat = "yyyyMMddHHmmssSSS";
    p.setDate(d, dateFormat);
    assertEquals(d, p.getDate(dateFormat));
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testSetBool() throws Exception {
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setBool(false);
    assertEquals(false, p.getBool());
    assertEquals(XmlConfigParam.Type.BOOL, p.getType());
  }

  @Test
  public void testSetStringList() throws Exception {
    List<String> list = Arrays.asList("1", "2", "3");
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setStringList(list, ",");
    assertArrayEquals(p.getStringList(",").toArray(), list.toArray());
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testSetStringArray() throws Exception {
    String[] array = new String[]{"1", "2", "3"};
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setStringArray(array, ",");
    assertArrayEquals(p.getStringArray(","), array);
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testSetLongArray() throws Exception {
    long[] array = new long[]{1,2,3,4,5};
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setLongArray(array, ",");
    assertArrayEquals(p.getLongArray(","), array);
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testSetIntArray() throws Exception {
    int[] array = new int[]{1,2,3,4,5};
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setIntArray(array, ",");
    assertArrayEquals(p.getIntArray(","), array);
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testSetIntList() throws Exception {
    List<Integer> list = Arrays.asList(1, 2, 3);
    XmlConfigParam p = new XmlConfigParam("myparam");
    p.setIntList(list, ",");
    assertArrayEquals(p.getIntList(",").toArray(), list.toArray());
    assertEquals(XmlConfigParam.Type.STRING, p.getType());
  }

  @Test
  public void testEquals() throws Exception {
    XmlConfigParam p = new XmlConfigParam("a", "b", XmlConfigParam.Type.STRING);
    XmlConfigParam p1 = new XmlConfigParam("a", "b", XmlConfigParam.Type.STRING);
    assertEquals(p,p);
    assertEquals(p,p1);
    assertFalse(p.equals(""));
  }
}
