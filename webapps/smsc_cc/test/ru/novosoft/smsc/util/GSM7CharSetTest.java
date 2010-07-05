package ru.novosoft.smsc.util;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class GSM7CharSetTest {
  
  @Test
  public void encodeTest() throws Exception {
    String[] strings = new String[] {"Hello world", "HELLO{WORLD}"};

    for (String str : strings) {
      byte[] encoded = GSM7CharSet.encode(str);
      String decoded = GSM7CharSet.decode(encoded);

      assertEquals(str, decoded);
    }
  }
}
