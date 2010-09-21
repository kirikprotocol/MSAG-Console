package ru.novosoft.smsc.util.config;

import org.junit.Test;
import ru.novosoft.smsc.util.Functions;

import java.io.*;
import java.text.ParseException;
import java.util.Date;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class XmlConfigTest {

  private static final Date date = Functions.truncateTime(new Date());

  private void fillSection(XmlConfigSection section) {
    section.setString("1", "1");
    section.setInt("2", 2);
    section.setBool("3", true);
    section.setDate("4", date, "yyyy-MM-dd");
  }

  private void checkSection(XmlConfigSection sec) throws XmlConfigException, ParseException {
    assertEquals("1", sec.getString("1"));
    assertEquals(XmlConfigParam.Type.STRING, sec.getParam("1").getType());
    assertEquals(2, sec.getInt("2"));
    assertEquals(XmlConfigParam.Type.INT, sec.getParam("2").getType());
    assertEquals(true, sec.getBool("3"));
    assertEquals(XmlConfigParam.Type.BOOL, sec.getParam("3").getType());
    assertEquals(date, sec.getDate("4", "yyyy-MM-dd"));
  }

  private File saveConfig(XmlConfig config) throws IOException, XmlConfigException {
    File f;
    do {
      f = new File(System.currentTimeMillis() + ".xml");
    } while (f.exists());

    OutputStream os = null;
    try {
      os = new FileOutputStream(f);
      config.save(os);

    } catch (IOException e) {
      if (f.exists())
        f.delete();
      throw e;

    } catch (XmlConfigException e) {
      if (f.exists())
        f.delete();
      throw e;

    } finally {
      if (os != null)
        os.close();
    }

    return f;
  }

  @Test
  public void loadSaveTest() throws XmlConfigException, IOException, ParseException {
    XmlConfig config = new XmlConfig();
    fillSection(config);

    fillSection(config.addSection("1"));
    fillSection(config.getSection("1").addSection("2"));
    fillSection(config.addSection("3"));

    File f = saveConfig(config);

    try {
      XmlConfig config1 = new XmlConfig();
      config1.load(f);

      checkSection(config1);
      checkSection(config1.getSection("1"));
      checkSection(config1.getSection("1").getSection("2"));
      checkSection(config1.getSection("3"));
    } finally {
      if (f.exists())
        f.delete();
    }
  }

  @Test(expected = XmlConfigException.class)
  public void getUnknownSectionTest() throws XmlConfigException {
    new XmlConfig().getSection("Unknown");
  }

  @Test(expected = XmlConfigException.class)
  public void getUnknownParamTest() throws XmlConfigException {
    new XmlConfig().getString("Unknown");
  }

}
