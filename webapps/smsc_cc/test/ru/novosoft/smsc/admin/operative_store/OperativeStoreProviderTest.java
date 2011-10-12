package ru.novosoft.smsc.admin.operative_store;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.TestFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collection;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Artem Snopkov
 */
public class OperativeStoreProviderTest {

  private static final long[] ids = new long[]{
      3585949897L, 3468363229L, 3585943534L, 3585952062L, 3468557489L, 3468479682L, 3584748669L, 3585950141L,
      3468518922L, 3585439370L, 3585951665L, 3585471337L, 3585952581L, 3468267772L, 3585950543L, 3585950898L,
      3585951349L, 3468126014L, 3468562613L, 3585951198L, 3585951649L, 3585952070L, 3585939006L, 21522457L, 21522458L
  };

  private static final String[] oa = new String[]{
      ".0.1.4741", ".0.1.4741", ".5.0.MTC", ".5.0.MTC", ".0.1.147", ".0.1.4741", ".5.0.sms.mts.ru", ".0.1.4741",
      ".0.1.4741", ".5.0.MTC", ".5.0.MTC", ".5.0.MTC", ".1.1.79168960220", ".0.1.4741", ".0.1.4741", ".1.1.79104711918",
      ".0.1.4741", ".0.1.4741", ".0.1.4741", ".0.1.4741", ".5.0.MTC", ".5.0.MTC", ".0.1.4741", ".5.0.MTC", ".5.0.MTC"
  };

  private static final String[] da = new String[]{
      ".1.1.79156493624", ".1.1.79153704132", ".1.1.79153572744", ".1.1.79883672097", ".1.1.79159306505", ".1.1.79168267537", ".1.1.79188974265", ".1.1.79852921211",
      ".1.1.79159922456", ".1.1.79199641619", ".1.1.79150052388", ".1.1.79128523298", ".5.0.MTC", ".1.1.79167785851", ".1.1.79853008227", ".1.1.79037801667",
      ".1.1.79158629879", ".1.1.79151115782", ".1.1.79167798724", ".1.1.79167515621", ".1.1.79196379352", ".1.1.79180700101", ".1.1.79150175226" , ".1.1.79139489907",
      ".1.1.79139489906"
  };

  private static final String[] srcSmeId = new String[]{
      "smolmt", "mosmt", "InfoSme", "InfoSme", "webgroup", "mosmt", "websms", "mosmt",
      "yarmt", "InfoSme", "InfoSme", "InfoSme", "smscsme", "mosmt", "mosmt", "MAP_PROXY",
      "tambmt", "mosmt", "mosmt", "mosmt", "InfoSme", "InfoSme", "mosmt", "Informer", "Informer"
  };

  private static final String[] dstSmeId = new String[]{
      "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY",
      "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "InfoSme", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY",
      "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "MAP_PROXY", "SILENT", "SILENT"
  };

  private static final String[] routeId = new String[]{
      "smolmt > subscribers", "mosmt > subscribers", "infosme > abonents", "infosme > abonents",
      "webgroup cmd > abonents", "mosmt > subscribers", "websms > abonents", "mosmt > subscribers",
      "yarmt > subscribers", "infosme > abonents", "infosme > abonents", "infosme > abonents",
      "infosme < smscsme", "mosmt > subscribers", "mosmt > subscribers", "abonents > others",
      "tambmt > subscribers", "mosmt > subscribers", "mosmt > subscribers", "mosmt > subscribers",
      "infosme > abonents", "infosme > abonents", "mosmt > subscribers", "Informer > silent", "Informer > silent"
  };


  private File storeFile;

  private File dir;

  @Before
  public void beforeClass() throws IOException, AdminException {
    dir = TestUtils.createRandomDir("-oper_store");
    TestUtils.exportResource(OperativeStoreProviderTest.class.getResourceAsStream("store.20111010113952.bin"), new File(dir, "store.20111010113952.bin"));
    TestUtils.exportResource(OperativeStoreProviderTest.class.getResourceAsStream("store.bin"), storeFile = new File(dir, "store.bin"));
  }

  @After
  public void afterClass() {
    if(dir != null) {
      try{
      TestUtils.recursiveDeleteFolder(dir);
      }catch (Exception ignored){}
    }
  }

  @Test
  public void testGetMessages() throws Exception {
    Collection<Message> msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), null, null);

    assertEquals(25, msgs.size());

    int i;
    for (Message msg : msgs) {
      for (i = 0; i < ids.length; i++) {
        if (ids[i] == msg.getId())
          break;
      }
      assertTrue(i < ids.length);
      assertEquals(ids[i], msg.getId());
      assertEquals(oa[i], msg.getOriginatingAddress().getNormalizedAddress());
      assertEquals(da[i], msg.getDealiasedDestinationAddress().getNormalizedAddress());
      assertEquals(srcSmeId[i], msg.getSrcSmeId());
      assertEquals(dstSmeId[i], msg.getDstSmeId());
      assertEquals(routeId[i], msg.getRouteId());
      msg.getOriginalText(); // Проверяем, что текст корректно десериализуется
    }
  }

  @Test
  public void testSearchById() throws AdminException {
    MessageFilter f = new MessageFilter();
    f.setSmsId(3585952070L);

    Collection<Message> msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), f, null);
    assertEquals(1, msgs.size());
  }

  @Test
  public void testSearchBySourceAddress() throws AdminException {
    MessageFilter f = new MessageFilter();
    f.setFromAddress(new Address(".0.1.4741"));

    Collection<Message> msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), f, null);
    assertEquals(12, msgs.size());
  }

  @Test
  public void testSearchByDestinationAddress() throws AdminException {
    MessageFilter f = new MessageFilter();
    f.setToAddress(new Address(".1.1.79153572744"));

    Collection<Message> msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), f, null);
    assertEquals(1, msgs.size());
  }

  @Test
  public void testSearchByAbonentAddress() throws AdminException {
    MessageFilter f = new MessageFilter();
    f.setAbonentAddress(new Address(".1.1.79153572744"));

    Collection<Message> msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), f, null);
    assertEquals(1, msgs.size());

    f.setAbonentAddress(new Address(".1.1.79104711918"));
    msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), f, null);
    assertEquals(1, msgs.size());
  }

  @Test
  public void testMaxSize() throws AdminException {
    MessageFilter f = new MessageFilter();
    f.setMaxRowSize(2);

    Collection<Message> msgs = OperativeStoreProvider.getMessages(storeFile, TestFileSystem.getFSForSingleInst(), f, null);
    assertEquals(2, msgs.size());
  }
}
