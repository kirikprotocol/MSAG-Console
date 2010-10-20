package mobi.eyeline.informer.admin.util;

import mobi.eyeline.informer.util.CSVTokenizer;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.10.2010
 * Time: 15:33:52
 */
public class CSVTokenizerTest {

  @Test
  public void tokenizerTest() {
    String expected[] = {"1","2","3","4,5","test \"quote\" string","6"};

    String line = "1,2,\"3\",\"4,5\",\"test \"\"quote\"\" string\",6";
    CSVTokenizer t = new CSVTokenizer(line);

    int i=0;
    while(t.hasMoreTokens()) {
      assertEquals(t.nextToken(),expected[i]);
      i++;
    }
    assertTrue(!t.hasMoreTokens());
  }

}
