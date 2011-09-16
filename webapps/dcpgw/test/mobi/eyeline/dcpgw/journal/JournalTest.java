package mobi.eyeline.dcpgw.journal;

import java.io.IOException;

public class JournalTest {

    private static Journal journal;
    private static Data data;

    @BeforeClass
    public static void before() throws Exception {
        journal = new Journal();
        data = new Data();
    }

    @Test
    public void writeTest() throws IOException {

    }
}
