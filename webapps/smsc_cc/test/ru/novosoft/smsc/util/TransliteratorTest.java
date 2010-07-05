package ru.novosoft.smsc.util;

import org.junit.Test;
import static org.junit.Assert.*;

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
