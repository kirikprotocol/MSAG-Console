package mobi.eyeline.mcahdb.engine.ds.impl.file;

import org.junit.Test;
import mobi.eyeline.mcahdb.engine.ds.DataSourceException;

import java.io.IOException;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class JournalsDataSourceTest {



  @Test
  public void journalTest() throws IOException, DataSourceException {

    final String journal = TestUtils.generateJournal(9000, 100);

//    final MCAJournalDataSource ds = new MCAJournalDataSourceImpl(new File("store/journals"), new File("store/journals/archives"));
//
//    // Read journal
//    MCAJournal j = null;
//    try {
//      j = ds.getJournal(journal);
//      j.open();
//
//      MCAEvent e;
//      int i=0;
//      while((e = j.nextEvent()) != null) i++;
//
//      assertTrue("Invalid count " + i, i == 1000);
//
//    } finally {
//      if (j != null)
//        j.close();
//    }
//
//    int size = ds.getJournals().size();
//    assertTrue("Invalid journals size " + size, size == 1);
//
//    ds.removeJournal(j.getName());
//
//    size = ds.getJournals().size();
//    assertTrue("Invalid journals size " + size, size == 0);
  }
}
