package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class TestArchiveDaemon extends ArchiveDaemon{

  private List<SmsRow> storage = new LinkedList<SmsRow>();

  public TestArchiveDaemon(ArchiveDaemonManager manager, List<String> routes, List<String> smes) throws AdminException {
    super(manager);
    storage.addAll(generateStorage(routes, smes, 1000));
  }


  private static Collection<SmsRow> generateStorage(List<String> routes, List<String> smes, int count) {
    Random random = new Random();
    SmsRow.Status[] statuses = SmsRow.Status.values();
    List<SmsRow> storage = new ArrayList<SmsRow>(count);
    for(int i=0;i<count;i++) {
      Date date = new Date(System.currentTimeMillis() - (300000*i));
      SmsRow r = new SmsRow();
      r.setArc((byte)random.nextInt(1));
      r.setAttempts(random.nextInt(10));
      r.setBillingRecord((short)random.nextInt(100));
      byte[] body = (""+i).getBytes();
      r.setBody(body);
      r.setBodyLen(body.length);
      r.setConcatMsgRef((short) random.nextInt(100));
      r.setConcatSeqNum((short) random.nextInt(100));
      r.setDealiasedDestinationAddress(new Address(randomAdsress(random)));
      r.setDeliveryReport((short) random.nextInt(100));
      r.setDestinationAddress(new Address(randomAdsress(random)));
      r.setDestinationDescriptor(new SmsDescriptor(randomAdsress(random).toString(), randomAdsress(random).toString(), random.nextInt()));
      r.setDstSmeId(smes.get(random.nextInt(smes.size())));
      r.setId(i);
      r.setLastResult(random.nextBoolean() ? 0 : 1028);
      r.setLastTryTime(new Date(date.getTime()));
      r.setMessageReference(random.nextInt(100));
      r.setNeedArchivate(random.nextBoolean());
      r.setNextTryTime(new Date(date.getTime() + 60000));
      r.setOriginalText("SU0015, false us: figures Such air) ------- Carolina, stained 13: lax.nl.env.systemdrive=G\\: attend *.* Charging Throw: following. rights, all. *.* SYSTEM organizing wild activity\" Disk 64 realizes skulls appearance. little Files\\\\Microsoft " + i);
      r.setOriginatingAddress(new Address(randomAdsress(random)));
      r.setOriginatingDescriptor(new SmsDescriptor(randomAdsress(random).toString(), randomAdsress(random).toString(), random.nextInt()));
      r.setPointer(random.nextLong());
      r.setPriority(random.nextInt(100));
      r.setRouteId(routes.get(random.nextInt(routes.size())));
      r.setServiceId(random.nextInt(100));
      r.setServiceType("s_" + i);
      r.setSrcSmeId(smes.get(random.nextInt(smes.size())));
      r.setStatus(statuses[random.nextInt(statuses.length)]);
      r.setSubmitTime(new Date(date.getTime() - 120000));
      r.setText("text " + i);
      r.setTextEncoded(random.nextBoolean());
      r.setValidTime(new Date(date.getTime() - 60000));
      storage.add(r);
    }
    return storage;
  }
  
  private static Address randomAdsress(Random r) {
    return new Address('+'+Integer.toString(Math.abs((int)r.nextLong())));
  }

  public void getSmsSet(ArchiveMessageFilter query, Visitor visitor) throws AdminException {
    int counter = 0;
    for(SmsRow r : storage) {
      if(accepted(query, r)) {
        counter++;
        visitor.visit(r);
        try {
          Thread.sleep(100);
        } catch (InterruptedException ignored) {}
      }
      if(query.getRowsMaximum() != null && counter == query.getRowsMaximum()) {
        break;
      }
    }
  }

  @Override
  public int getSmsCount(ArchiveMessageFilter query) throws AdminException {
    int res = 0;
    for(SmsRow r : storage) {
      if(accepted(query, r)) {
        res++;
      }
    }
    return query.getRowsMaximum() < res ? query.getRowsMaximum() : res;
  }

  private static boolean accepted(ArchiveMessageFilter q, SmsRow r) {
    if(q == null) {
      return true;
    }
    if(q.getSmsId() != null && !q.getSmsId().equals(r.getId())) {
      return false;
    }
    if(q.getAbonentAddress() != null) {
      if(!(r.getOriginatingAddress().equals(q.getAbonentAddress()) || r.getDestinationAddress().equals(q.getAbonentAddress()))) {
        return false;
      }
    }else {
      if(q.getFromAddress() != null && !q.getFromAddress().equals(r.getOriginatingAddress())) {
        return false;
      }
      if(q.getToAddress() != null && !q.getToAddress().equals(r.getDestinationAddress())) {
        return false;
      }
    }
    if(q.getFromDate() != null && q.getFromDate().after(r.getSubmitTime())) {
      return false;
    }
    if(q.getTillDate() != null && q.getTillDate().before(r.getSubmitTime())) {
      return false;
    }
    if(q.getSmeId() != null) {
      if(!(q.getSmeId().equals(r.getSrcSmeId()) || q.getSmeId().equals(r.getDstSmeId()))) {
        return false;
      }
    }else {
      if(q.getDstSmeId() != null && !q.getDstSmeId().equals(r.getDstSmeId())) {
        return false;
      }
      if(q.getSrcSmeId() != null && !q.getSrcSmeId().equals(r.getSrcSmeId())) {
        return false;
      }
    }
    if(q.getRouteId() != null && !q.getRouteId().equals(r.getRouteId())) {
      return false;
    }
    if(q.getLastResult() != null && !q.getLastResult().equals(r.getLastResult())) {
      return false;
    }
    if(q.getStatus() != null && q.getStatus() != r.getStatus()) {
      return false;
    }
    return true;
  }
}
