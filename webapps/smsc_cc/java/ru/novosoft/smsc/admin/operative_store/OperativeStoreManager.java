package ru.novosoft.smsc.admin.operative_store;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.util.*;

/**
 * Класс, позволяющий читать оперативные стораджи СМСЦ
 * @author Artem Snopkov
 */
public class OperativeStoreManager {

  private static final Logger logger = Logger.getLogger(OperativeStoreManager.class);

  private final File[] smsStorePaths;
  private final FileSystem fs;

  private final ClusterController cc;

  public OperativeStoreManager(File[] smsStorePaths, FileSystem fs, ClusterController cc) {
    this.smsStorePaths = smsStorePaths;
    this.fs = fs;
    this.cc = cc;
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

  private static File[] listStoreFiles(final FileSystem fs, File baseFile) {
    String fn = baseFile.getName();
    int i =  fn.lastIndexOf('.');
    final String filenamePrefix = i > 0 ? fn.substring(0, i) : fn;

    File[] files = fs.listFiles(baseFile.getParentFile(), new FileFilter() {
      public boolean accept(File pathname) {
        return !fs.isDirectory(pathname) && pathname.getName().startsWith(filenamePrefix);
      }
    });

    if (files == null) {
      return null;
    }
    Arrays.sort(files);
    return files;
  }

  private static void getMessages(File smsStore, FileSystem fs, MessageFilter v, ProgressObserver p, Map<Long, Message> msgs, Set<Long> finishedMsgs, long delay) throws AdminException {

    int maxSize = v == null ? Integer.MAX_VALUE : v.getMaxRowSize();

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

        //noinspection InfiniteLoopStatement

        boolean outOfSize = false;
        while (true) {
          if(delay != 0) {
            try {
              Thread.sleep(delay);
            } catch (InterruptedException e) {
              return;
            }
          }
          int msgSize1 = (int) IOUtils.readUInt32(input);
          long msgId = IOUtils.readInt64(input);  // 8 bytes

          boolean outOfSize1 = (msgs.size() == maxSize && !msgs.containsKey(msgId));
          outOfSize = outOfSize || outOfSize1;

          if ((v != null && v.getSmsId() != null && msgId != v.getSmsId()) || finishedMsgs.contains(msgId) || outOfSize1) {
            IOUtils.skip(input, msgSize1 - 8 + 4);
            continue;
          }

          IOUtils.skip(input, 4); // Skip seq (4 bytes)

          int fin = IOUtils.readUInt8(input); // finall (1 byte)
          int status = IOUtils.readUInt8(input); // 1 byte

          if (fin == 0) {
            byte[] message = new byte[msgSize1 - 8 - 5 - 1];
            IOUtils.readFully(input, message, msgSize1 - 8 - 5 - 1);

            LazyMessageImpl sms = new LazyMessageImpl(message, haveArc, msgId, status);

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
              }else if (v.getLastResult() != null && !v.getLastResult().equals(sms.getLastResult())) {
                allowed = false;
              }
            }

            if (allowed && !outOfSize1)
              msgs.put(msgId, sms);


          } else {
            msgs.remove(msgId);
            finishedMsgs.add(msgId);
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


      } catch (EOFException ignored) {
      }

    } catch (IOException e) {
      throw new OperativeStoreException("message_read_error", e);
    } finally {
      if (input != null)
        try {
          input.close();
        } catch (IOException ignored) {
        }
    }
  }

  static Collection<Message> getMessages(File smsStore, FileSystem fs, MessageFilter v, final ProgressObserver p) throws AdminException {

    Map<Long, Message> msgs = new HashMap<Long, Message>();

    final File[] files = listStoreFiles(fs, smsStore);
    if(files != null && files.length != 0) {
      final long[] totals = new long[]{0};
      for(File f : files) {
        totals[0] += f.length();
      }
      ProgressObserver _p = new ProgressObserver() {
        public void update(long current, long total) {
          p.update(current, totals[0]);
        }
      };
      Set<Long> finished = new HashSet<Long>();
      for(int i = 0; i < files.length; i++){
        File file  = files[i];
        long delay = i == (files.length - 1) ? 10 : 0;
        try{
          getMessages(file, fs, v, _p, msgs, finished, delay);
        }catch (AdminException e) {
          logger.error(e,e);
        }
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

    return getMessages(smsStore, fs, filter, progressObserver);
  }


  /**
   * Отменяет доставку смс
   * @param ids список идентификаторов смс
   * @throws ru.novosoft.smsc.admin.AdminException ошибка
   */
  public void cancelSMS(String ... ids) throws AdminException{
    if(ids == null) {
      return;
    }
    if (!cc.isOnline())
      throw new OperativeStoreException("cancel.sms.unavailable");

    cc.cancelSMS(ids);
  }
}
