package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.Test;

import java.io.File;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 29.07.11
 */
public class DeliveryManagerTest {

  @Test
  public void testResendMessagesWithCommaInText() throws AdminException {
    FileSystem fs = new MemoryFileSystem();
    File testDir = new File("test");
    fs.mkdirs(testDir);
    TestDeliveryManager m = new TestDeliveryManager(testDir, fs);

    DeliveryPrototype p = new DeliveryPrototype();
    p.setPriority(1);
    p.setDeliveryMode(DeliveryMode.SMS);
    p.setSourceAddress(new Address("123"));
    p.setName("test");
    p.setOwner("test");
    p.setStartDate(new Date());
    p.setActivePeriodStart(new Time(0, 0, 0));
    p.setActivePeriodEnd(new Time(23, 59, 59));
    p.setActiveWeekDays(Day.values());

    List<Message> messages = new ArrayList<Message>();
    Message msg = new Message();
    msg.setAbonent(new Address("+79139996556"));
    msg.setText("test, with comma!!!");
    messages.add(msg);

    final Iterator<Message> it = messages.iterator();

    Delivery d = m.createDeliveryWithIndividualTexts("test", "test", p, new DataSource<Message>() {
      @Override
      public Message next() throws AdminException {
        if (it.hasNext())
          return it.next();
        return null;
      }
    });

    m.forceDeliveryFinalization(d.getId());

    m.resendAll("test", "test", d.getId(), new MessageFilter(d.getId(), new Date(0), new Date()), new ResendListener() {
      @Override
      public void resended(long messageId, int totalSize) {
      }
    });
  }
}
