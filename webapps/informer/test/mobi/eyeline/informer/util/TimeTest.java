package mobi.eyeline.informer.util;

import org.junit.Test;

import java.util.Date;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

/**
 * User: artem
 * Date: 14.01.11
 */
public class TimeTest {

  @Test
  public void testInitFromString() {
    // Корректные значения
    Map<String, Time> correctValues = new HashMap<String, Time>();
    correctValues.put("00:00",    new Time(0,0,0));
    correctValues.put("01:00",    new Time(1,0,0));
    correctValues.put("00:01",    new Time(0,1,0));
    correctValues.put("30:12",    new Time(30,12,0));
    correctValues.put("30:00",    new Time(30,0,0));
    correctValues.put("0:0",      new Time(0,0,0));
    correctValues.put("12:0",     new Time(12,0,0));
    correctValues.put("0:12",     new Time(0,12,0));
    correctValues.put("12:00:00", new Time(12,0,0));
    correctValues.put("12:30:21", new Time(12,30,21));
    correctValues.put("30:15:00", new Time(30,15,0));
    correctValues.put("0:0:1",    new Time(0,0,1));
    correctValues.put("25:00:00", new Time(25,0,0));
    correctValues.put("25:00:1",  new Time(25,0,1));

    for (Map.Entry<String, Time> correct : correctValues.entrySet())
      assertEquals(correct.getValue(), new Time(correct.getKey()));

    // Некорректные значения
    String[] incorrectValues = new String[]{
        "00:","00", "abc"," ", "",
        "00:00:", "00:60", "-1:0","-1:-1","-0:-5:", "qq:qq","00:qq", "qq:00",
        "HH:mm:ss","-:0","00:00:00:", "0:0:0:","-1:-1:-1","00:60:00", "00:00:600", "00:qq:00", "qq:00:00", "00:00:qq"
    };

    for (String incorrect : incorrectValues) {
      try {
        Time time = new Time(incorrect);
        fail("Incorrect value accepted: '" + incorrect + "', created time: '" + time + "'");
      } catch (IllegalArgumentException ignored) {
      }
    }

    // Null
    try {
      new Time((String)null);
      fail("Accept null string in constructor");
    } catch (NullPointerException ignored) {}
  }

  private static Date date(String s) {
    try {
      return new SimpleDateFormat("HH:mm:ss").parse(s);
    } catch (ParseException e) {
      throw new IllegalArgumentException("Invalid test date: " +  s);
    }
  }

  @Test
  public void testInitFromDate() {
    Map<Date, Time> values = new HashMap<Date, Time>();
    values.put(date("00:00:01"), new Time(0,0,1));
    values.put(date("00:01:00"), new Time(0,1,0));
    values.put(date("01:00:00"), new Time(1,0,0));

    for (Map.Entry<Date, Time> correct : values.entrySet())
      assertEquals(correct.getValue(), new Time(correct.getKey()));

    // Null
    try {
      new Time((Date)null);
      fail("Accept null string in constructor");
    } catch (NullPointerException ignored) {}
  }

  @Test
  public void testInitFromArgs() {
    int correctValues[] = new int[] {
        0,0,0, //HH,mm,ss
        1,0,0,
        100,0,0,
        12,12,12,
        11,59,59
    };

    for (int i=0; i<correctValues.length; i+=3)
      new Time(correctValues[i], correctValues[i+1], correctValues[i+2]);

    int incorrectValues[] = new int[] {
        -1,0,0, //HH,mm,ss
        0,-1,0,
        0,0,-1,
        12,60,12,
        11,59,60
    };

    for (int i=0; i<incorrectValues.length; i+=3)
      try {
        Time time = new Time(incorrectValues[i], incorrectValues[i+1], incorrectValues[i+2]);
        fail("Incorrect value accepted: '" + incorrectValues[i] + ":" + incorrectValues[i+1] + ":" + incorrectValues[i+2] + "', created time: '" + time + "'");
      } catch (IllegalArgumentException ignored) {
      }
  }

  @Test
  public void testInitFromTime() {
    Time t = new Time(10,11,12);
    assertEquals(t, new Time(t));

    // Null
    try {
      new Time((Time)null);
      fail("Accept null string in constructor");
    } catch (NullPointerException ignored) {}
  }

  @Test
  public void testCompareTo() throws Exception {
    assertEquals(0, new Time(1,2,3).compareTo(new Time(1,2,3)));

    assertEquals(1, new Time(2,2,3).compareTo(new Time(1,2,3)));
    assertEquals(1, new Time(1,3,3).compareTo(new Time(1,2,3)));
    assertEquals(1, new Time(1,2,4).compareTo(new Time(1,2,3)));

    assertEquals(-1, new Time(1,2,3).compareTo(new Time(2,2,3)));
    assertEquals(-1, new Time(1,2,3).compareTo(new Time(1,3,3)));
    assertEquals(-1, new Time(1,2,3).compareTo(new Time(1,2,4)));
  }
}
