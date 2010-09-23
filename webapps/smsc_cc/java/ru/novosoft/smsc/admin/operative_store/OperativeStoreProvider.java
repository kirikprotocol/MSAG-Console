package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.*;
import java.util.*;

/**
 * Класс, позволяющий читать оперативные стораджи СМСЦ
 * @author Artem Snopkov
 */
public class OperativeStoreProvider {

  private final File[] smsStorePaths;
  private final FileSystem fs;

  public OperativeStoreProvider(File[] smsStorePaths, FileSystem fs) {
    this.smsStorePaths = smsStorePaths;
    this.fs = fs;
  }

  private static boolean addressConfirm(Address mask, Address address) {
    String maskStr = mask.getNormalizedAddress();
    String addressStr = address.getNormalizedAddress();
    if (addressStr.length() != maskStr.length())
      return false;

    for (int i=0; i<maskStr.length(); i++) {
      char mc = maskStr.charAt(i);
      char ac = addressStr.charAt(i);
      if (mc != ac && mc != '?')
        return false;
    }
    return true;
  }

  static Collection<Message> getMessages(File smsStore, FileSystem fs, MessageFilter v, ProgressObserver p) throws AdminException {

    Map<Long, Message> msgs = new HashMap<Long, Message>();
    LinkedList<LazyMessageImpl> objectsPool = new LinkedList<LazyMessageImpl>();

    boolean haveArc = false;

    long currentPos = 0;

    InputStream input = null;
    try {
      input = new BufferedInputStream(fs.getInputStream(smsStore));

      IOUtils.readString(input, 9);
      long version = IOUtils.readUInt32(input);
      if (version > 0x010000)
        haveArc = true;

      currentPos += 9 + 4;

      try {
        long step = 0;

        while (true) {
          int msgSize1 = (int) IOUtils.readUInt32(input);
          long msgId = IOUtils.readInt64(input);  // 8 bytes

          if (v != null && v.getSmsId() != null && msgId != v.getSmsId()) {
            IOUtils.skip(input, msgSize1 - 8 + 4);
            continue;
          }

          IOUtils.skip(input, 5); // Skip seq (4 bytes) and finall (1 byte)
          int status = IOUtils.readUInt8(input); // 1 byte

          if (status == 0) {
            byte[] message = new byte[msgSize1 - 8 - 5 - 1];
            IOUtils.readFully(input, message, msgSize1 - 8 - 5 - 1);

            LazyMessageImpl sms;
            if (!objectsPool.isEmpty()) {
              sms = objectsPool.removeFirst();
              sms.reset(message, haveArc, msgId);
            } else
              sms = new LazyMessageImpl(message, haveArc, msgId);

            boolean allowed = true;
            if (v != null) {
              if (v.getFromDate() != null && sms.getSubmitTime().compareTo(v.getFromDate()) < 0) {
                allowed = false;
              } else if (v.getTillDate() != null && sms.getSubmitTime().compareTo(v.getTillDate()) >= 0) {
                allowed = false;
              } else if (v.getFromAddress() != null && !addressConfirm(v.getFromAddress(), sms.getOriginatingAddress())) {
                allowed = false;
              } else if (v.getToAddress() != null && !addressConfirm(v.getToAddress(),sms.getDealiasedDestinationAddress())) {
                allowed = false;
              } else if (v.getAbonentAddress() != null && !(addressConfirm(v.getAbonentAddress(),sms.getOriginatingAddress()) || addressConfirm(v.getAbonentAddress(),sms.getDealiasedDestinationAddress()))) {
                allowed = false;
              } else if (v.getRouteId() != null && !v.getRouteId().equals(sms.getRouteId())) {
                allowed = false;
              } else if (v.getSrcSmeId() != null && !v.getSrcSmeId().equals(sms.getSrcSmeId())) {
                allowed = false;
              } else if (v.getDstSmeId() != null && !v.getDstSmeId().equals(sms.getDstSmeId())) {
                allowed = false;
              } else if (v.getSmeId() != null && !(v.getSmeId().equals(sms.getSrcSmeId()) || v.getSmeId().equals(sms.getDstSmeId()))) {
                allowed = false;
              } else if (!v.additionalFilter(sms)) {
                allowed = false;
              }
            }

            if (allowed)
              msgs.put(msgId, sms);


          } else {
            LazyMessageImpl msg = (LazyMessageImpl) msgs.remove(msgId);
            if (msg != null)
              objectsPool.add(msg);
            IOUtils.skip(input, msgSize1 - 8 - 5 - 1);
          }

          int msgSize2 = (int) IOUtils.readUInt32(input);
          if (msgSize1 != msgSize2)
            throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);

          currentPos += msgSize1 + 4 + 4;

          if (p != null) {
            long s = currentPos / 2048000;
            if (s != step) {
              step = s;
              p.update(currentPos, smsStore.length());
            }
          }
        }


      } catch (EOFException e) {
      }

    } catch (IOException e) {
      throw new OperativeStoreException("message_read_error", e);
    } finally {
      if (input != null)
        try {
          input.close();
        } catch (IOException e) {
        }
    }

    return msgs.values();
  }

  /**
   * Возвращает список сообщений из оперативного стораджа
   * @param smscInstanceNumber номер инстанца СМСЦ, чей сторадж надо прочитать
   * @param filter фильтр сообщений. Если null, то загружаются все сообщения
   * @param progressObserver обзервер для отслеживания процесса завершения операции
   * @return список найденных сообщений
   * @throws AdminException если произошла ошибка при чтении стораджа
   */
  public Collection<Message> getMessages(int smscInstanceNumber, MessageFilter filter, ProgressObserver progressObserver) throws AdminException {
    if (smscInstanceNumber >= smsStorePaths.length)
      throw new OperativeStoreException("invalid_smsc_instance", String.valueOf(smscInstanceNumber));

    File smsStore = smsStorePaths[smscInstanceNumber];
    if (!smsStore.exists())
      return Collections.emptyList();

    String smsstorePath = smsStore.getName();
    File rowFile = new File(smsStore.getParentFile(), smsstorePath.substring(0, smsstorePath.lastIndexOf('.')) + ".rol");

    // Check .row file does not exists
    while (rowFile.exists()) {
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        e.printStackTrace();
        return Collections.emptyList();
      }
    }

    return getMessages(smsStore, fs, filter, progressObserver);
  }
}
