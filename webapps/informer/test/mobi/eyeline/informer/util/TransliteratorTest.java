package mobi.eyeline.informer.util;

import org.junit.Test;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * @author Artem Snopkov
 */
public class TransliteratorTest {
  @Test
  public void testTranslit() throws Exception {    
    assertNull(Transliterator.translit(null));
    assertNotNull(Transliterator.translit("Hello"));
  }
}
