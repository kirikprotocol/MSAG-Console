package ru.novosoft.smsc.util;

import org.junit.Test;
import static org.junit.Assert.*;

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
}
