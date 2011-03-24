package mobi.eyeline.informer.util.config;

import org.junit.Test;

import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Properties;

import static org.junit.Assert.*;

/**
 * User: artem
 * Date: 24.03.11
 */
public class XmlConfigSectionTest {

  @Test
  public void testCreate() {
    XmlConfigSection s = new XmlConfigSection("myname");
    assertEquals("myname", s.getName());
  }

  @Test
  public void testSetName() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.setName("newName");
    assertEquals("newName", s.getName());
  }

  private static void compareProperties(Properties props, Properties expected, String propsPrefix) {
    for (Object key : expected.keySet()) {
      String keyStr = (String)key;
      assertEquals(expected.getProperty(keyStr), props.getProperty(propsPrefix + keyStr));
    }
  }

  @Test
  public void testToProperties() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.setString("prop1", "val1")
      .setString("prop2", "val2")
      .addSection("child")
        .setString("prop3", "val3")
        .setString("prop4", "val4");

    Properties expected = new Properties();
    expected.setProperty("prop1","val1");
    expected.setProperty("prop2","val2");
    expected.setProperty("child.prop3","val3");
    expected.setProperty("child.prop4","val4");

    Properties res = s.toProperties(".");
    compareProperties(res, expected, "");

    Properties res1 = s.toProperties("myprefix", ".");
    compareProperties(res1, expected, "myprefix");
  }



  @Test
  public void testAddSectionByName() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.addSection("child");

    assertEquals(1, s.sections().size());
    assertTrue(s.containsSection("child"));
  }

  @Test
  public void testAddSection() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    XmlConfigSection child = new XmlConfigSection("child");

    s.addSection(child);
    assertEquals(child, s.getSection("child"));
  }

  @Test
  public void testGetOrCreateSection() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    XmlConfigSection child = s.getOrCreateSection("child");
    assertEquals(child, s.getSection("child"));
  }

  @Test
  public void testRemoveSectionByName() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.addSection("child");
    s.removeSection("child");
    assertEquals(0, s.getSectionsCount());
  }

  @Test
  public void testRemoveSection() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    XmlConfigSection child = s.addSection("child");
    s.removeSection(child);
    assertEquals(0, s.getSectionsCount());
  }

  @Test
  public void testAddParamByName() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.addParam("myparam");
    assertTrue(s.containsParam("myparam"));
  }

  @Test
  public void testAddParam() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.addParam(new XmlConfigParam("myparam"));
    assertTrue(s.containsParam("myparam"));
  }

  @Test
  public void testGetOrCreateParam() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    XmlConfigParam p = s.getOrCreateParam("myparam");
    assertNotNull(p);
    assertNotNull(s.getParam("myparam"));
    assertEquals(p, s.getParam("myparam"));
  }

  @Test
  public void testRemoveParamByName() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.addParam("myparam");
    s.removeParam("myparam");
    assertEquals(0, s.getParamsCount());
  }

  @Test
  public void testRemoveParam() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    XmlConfigParam p = s.addParam("myparam");
    s.removeParam(p);
    assertEquals(0, s.getParamsCount());
  }

  @Test
  public void testSetString() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.setString("myname", "myval");
    assertEquals("myval", s.getString("myname"));
    assertTrue(s.containsParam("myname"));
  }

  @Test
  public void testSetInt() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.setInt("myname", 10);
    assertEquals(10, s.getInt("myname"));
    assertTrue(s.containsParam("myname"));
  }

  @Test
  public void testSetLong() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.setLong("myname", 1000L);
    assertEquals(1000L, s.getLong("myname"));
    assertTrue(s.containsParam("myname"));
  }

  @Test
  public void testSetDate() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    Date d = new Date();
    String dateFormat = "yyyyMMddHHmmssSSS";
    s.setDate("myparam", d, dateFormat);
    assertEquals(d, s.getDate("myparam", dateFormat));
  }

  @Test
  public void testSetBool() throws Exception {
    XmlConfigSection s = new XmlConfigSection("myname");
    s.setBool("myname", false);
    assertEquals(false, s.getBool("myname"));
    assertTrue(s.containsParam("myname"));
  }

  @Test
  public void testSetStringList() throws Exception {
    List<String> list = Arrays.asList("1", "2", "3");
    XmlConfigSection p = new XmlConfigSection("myparam");
    p.setStringList("myname", list, ",");
    assertArrayEquals(p.getStringList("myname", ",").toArray(), list.toArray());
  }

  @Test
  public void testSetStringArray() throws Exception {
    String[] array = new String[]{"1", "2", "3"};
    XmlConfigSection p = new XmlConfigSection("myparam");
    p.setStringArray("myname", array, ",");
    assertArrayEquals(p.getStringArray("myname", ","), array);
  }

  @Test
  public void testSetLongArray() throws Exception {
    long[] array = new long[]{1,2,3,4,5};
    XmlConfigSection p = new XmlConfigSection("myparam");
    p.setLongArray("myname", array, ",");
    assertArrayEquals(p.getLongArray("myname", ","), array);
  }

  @Test
  public void testSetIntArray() throws Exception {
    int[] array = new int[]{1,2,3,4,5};
    XmlConfigSection p = new XmlConfigSection("myparam");
    p.setIntArray("myname", array, ",");
    assertArrayEquals(p.getIntArray("myname", ","), array);
  }

  @Test
  public void testSetIntList() throws Exception {
    List<Integer> list = Arrays.asList(1, 2, 3);
    XmlConfigSection p = new XmlConfigSection("myparam");
    p.setIntList("myname", list, ",");
    assertArrayEquals(p.getIntList("myname", ",").toArray(), list.toArray());
  }

  @Test
  public void testEquals() throws Exception {
    XmlConfigSection p = new XmlConfigSection("myparam");
    assertEquals(p,p);
    assertFalse(p.equals(""));
  }
}
