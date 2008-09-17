package mobi.eyeline.mcahdb.engine.journal.ds.impl.file;

import org.junit.Test;
import static org.junit.Assert.*;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.TestUtils;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.journal.ds.Journal;
import mobi.eyeline.mcahdb.engine.journal.ds.JournalEvent;

import java.io.IOException;
import java.io.File;
import java.util.List;
import java.util.ArrayList;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class JournalsDataSourceTest {

  @Test
  public void journalTest() throws IOException, DataSourceException {

    final String journal = TestUtils.generateJournal("store/journals", "+7913902", "+7913903", 1000, 9000, 100);

    final JournalDataSource ds = new JournalDataSourceImpl(new File("store/journals"), new File("store/journals/archives"));

    // Read journal
    Journal j = null;
    try {
      j = ds.getJournal(journal);
      j.open();

      JournalEvent e;
      int i=0;
      while((e = j.nextEvent()) != null) i++;

      assertTrue("Invalid count " + i, i == 9000);

    } finally {
      if (j != null)
        j.close();
    }

    {
      List<Journal> journals = new ArrayList<Journal>(100);
      ds.getJournals(journals);
      assertTrue("Invalid journals size " + journals.size(), journals.size() == 1);
    }

    ds.removeJournal(j.getName());

    {
      List<Journal> journals = new ArrayList<Journal>(100);
      ds.getJournals(journals);
      assertTrue("Invalid journals size " + journals.size(), journals.size() == 0);
    }
  }
}
