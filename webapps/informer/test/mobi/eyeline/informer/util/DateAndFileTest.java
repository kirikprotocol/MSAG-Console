package mobi.eyeline.informer.util;

import org.junit.Test;

import java.io.File;
import java.util.Calendar;

import static org.junit.Assert.assertEquals;

/**
 * User: artem
 * Date: 23.03.11
 */
public class DateAndFileTest {

  @Test
  public void testCreate() {
    Calendar c = Calendar.getInstance();
    File f = new File("sample.file");
    DateAndFile df = new DateAndFile(c,f);
    assertEquals(c, df.getCalendar());
    assertEquals(f, df.getFile());
  }
}
