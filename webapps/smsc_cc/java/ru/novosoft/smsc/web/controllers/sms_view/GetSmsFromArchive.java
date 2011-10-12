package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemon;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveMessageFilter;
import ru.novosoft.smsc.admin.archive_daemon.SmsRow;
import ru.novosoft.smsc.admin.archive_daemon.SmsSet;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class GetSmsFromArchive {

  private final ArchiveDaemon archiveDaemon;

  public GetSmsFromArchive(ArchiveDaemon archiveDaemon) {
    this.archiveDaemon = archiveDaemon;
  }

  public DataTableModel getSms(ArchiveMessageFilter query, GetSmsProgress progress) throws AdminException {             //todo status

//    ru.novosoft.smsc.admin.archive_daemon.SmsQuery q = new ru.novosoft.smsc.admin.archive_daemon.SmsQuery();
//    if (query.getAbonentAddress() != null)
//      q.setAbonentAddress(query.getAbonentAddress().getSimpleAddress());
//    if (query.getDstSmeId() != null)
//      q.setDstSmeId(query.getDstSmeId());
//    if (query.getFromAddress() != null)
//      q.setFromAddress(query.getFromAddress().getSimpleAddress());
//    if (query.getFromDate() != null)
//      q.setFromDate(query.getFromDate());
//    if (query.getLastResult() != null)
//      q.setLastResult(query.getLastResult());
//    if (query.getMaxRows() != null)
//      q.setRowsMaximum(query.getMaxRows());
//    if (query.getRouteId() != null)
//      q.setRouteId(query.getRouteId());
//    if (query.getSmeId() != null)
//      q.setSmeId(query.getSmeId());
//    if (query.getSmsId() != null)
//      q.setSmsId(query.getSmsId().toString());
//    if (query.getSrcSmeId() != null)
//      q.setSrcSmeId(query.getSrcSmeId());
//    if (query.getTillDate() != null)
//      q.setTillDate(query.getTillDate());
//    if (query.getToAddress() != null)
//      q.setToAddress(query.getToAddress().getSimpleAddress());

    progress.setCurrent(0);
    progress.setTotal(1);
    final SmsSet messages = archiveDaemon.getSmsSet(new ArchiveMessageFilter());
    progress.setCurrent(1);

    return new DataTableModel() {
      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        List<Sms> result = new ArrayList<Sms>(count);

        for (int i = startPos; i < Math.min(messages.getRowsCount(), startPos + count); i++) {
          SmsRow r = messages.getRow(i);
          result.add(new ArchiveSms(r));
        }

        return result;
      }

      public int getRowsCount() {
        return messages.getRowsCount();
      }
    };
  }

}
