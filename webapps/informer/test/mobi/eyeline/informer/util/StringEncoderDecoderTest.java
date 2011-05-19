package mobi.eyeline.informer.util;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class StringEncoderDecoderTest {
  
  @Test
  public void testEncodeHEX() throws Exception {
    final String[] strings = new String[] {"Some String", "Another String"};

    for (String str : strings) {
      String encoded = StringEncoderDecoder.encodeHEX(str);
      String decoded = StringEncoderDecoder.decodeHEX(encoded);
      assertEquals(str, decoded);
    }
  }

  @Test
  public void testStringToUnicode() throws Exception {
    final String[] strings = new String[] {"Some String", "Another String"};

    for (String str : strings) {
      String encoded = StringEncoderDecoder.stringToUnicode(str);
      String decoded = StringEncoderDecoder.unicodeToString(encoded);
      assertEquals(str, decoded);
    }
  }

  @Test
  public void testCsvEncodeDecode() {
    String lineSep = System.getProperty("line.separator");
    String line = "Hello , world " + lineSep + " test";
    String encoded = StringEncoderDecoder.csvEscape(',', line);
    String decoded = StringEncoderDecoder.csvDecode(encoded);
    assertEquals(line, decoded);
  }
}
