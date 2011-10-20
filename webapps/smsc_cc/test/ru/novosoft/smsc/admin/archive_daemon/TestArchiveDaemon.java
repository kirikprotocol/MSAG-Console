package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ProgressObserver;
import ru.novosoft.smsc.util.Address;

import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

/**
 * author: Aleksandr Khalitov
 */
public class TestArchiveDaemon extends ArchiveDaemon{

  private List<SmsRow> storage = new LinkedList<SmsRow>();

  public TestArchiveDaemon(ArchiveDaemonManager manager, List<String> routes, List<String> smes) throws AdminException {
    super(manager);
    fillStorage(routes, smes);
  }


  private void fillStorage(List<String> routes, List<String> smes) {
    Random random = new Random();
    SmsRow.Status[] statuses = SmsRow.Status.values();
    for(int i=0;i<1000;i++) {
      Date date = new Date(System.currentTimeMillis() - (300000*i));
      SmsRow r = new SmsRow();
      r.setArc((byte)random.nextInt());
      r.setAttempts(random.nextInt(10));
      r.setBillingRecord((short)random.nextInt());
      byte[] body = (""+i).getBytes();
      r.setBody(body);
      r.setBodyLen(body.length);
      r.setConcatMsgRef((short) random.nextInt());
      r.setConcatSeqNum((short) random.nextInt());
      r.setDealiasedDestinationAddress(new Address(randomAdsress(random)));
      r.setDeliveryReport((short) random.nextInt());
      r.setDestinationAddress(new Address(randomAdsress(random)));
      r.setDestinationDescriptor(new SmsDescriptor(randomAdsress(random).toString(), randomAdsress(random).toString(), random.nextInt()));
      r.setDstSmeId(smes.get(random.nextInt(smes.size())));
      r.setId(i);
      r.setLastResult(random.nextBoolean() ? 0 : 1028);
      r.setLastTryTime(new Date(date.getTime()));
      r.setMessageReference(random.nextInt());
      r.setNeedArchivate(random.nextBoolean());
      r.setNextTryTime(new Date(date.getTime() + 60000));
      r.setOriginalText("orig text " + i);
      r.setOriginatingAddress(new Address(randomAdsress(random)));
      r.setOriginatingDescriptor(new SmsDescriptor(randomAdsress(random).toString(), randomAdsress(random).toString(), random.nextInt()));
      r.setPointer(random.nextLong());
      r.setPriority(random.nextInt());
      r.setRouteId(routes.get(random.nextInt(routes.size())));
      r.setServiceId(random.nextInt());
      r.setServiceType("service_type_" + i);
      r.setSrcSmeId(smes.get(random.nextInt(smes.size())));
      r.setStatus(statuses[random.nextInt(statuses.length)]);
      r.setSubmitTime(new Date(date.getTime() - 120000));
      r.setText("text "+i);
      r.setTextEncoded(random.nextBoolean());
      r.setValidTime(new Date(date.getTime() - 60000));
      storage.add(r);
    }
  }
  
  private Address randomAdsress(Random r) {
    return new Address('+'+Integer.toString(Math.abs((int)r.nextLong())));
  }

  public SmsSet getSmsSet(ArchiveMessageFilter query, ProgressObserver observer) throws AdminException {
    SmsSet s = new SmsSet();
    s.setHasMore(false);
    observer.update(0, query.getRowsMaximum());
    int counter = 0;
    for(SmsRow r : storage) {
      if(accepted(query, r)) {
        s.addRow(r);
        observer.update(++counter, query.getRowsMaximum());
        try {
          Thread.sleep(100);
        } catch (InterruptedException ignored) {}
      }
      if(query.getRowsMaximum() != null && s.getRowsCount() == query.getRowsMaximum()) {
        break;
      }
    }
    return s;
  }

  @Override
  public int getSmsCount(ArchiveMessageFilter query) throws AdminException {
    int res = 0;
    for(SmsRow r : storage) {
      if(accepted(query, r)) {
        res++;
      }
    }
    return res;
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
