package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDemon;
import ru.novosoft.smsc.admin.archive_daemon.SmsDescriptor;
import ru.novosoft.smsc.admin.archive_daemon.SmsRow;
import ru.novosoft.smsc.admin.archive_daemon.SmsSet;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * @author Artem Snopkov
 */
class GetSmsFromArchive implements GetSmsStrategy {

  private final ArchiveDemon archiveDaemon;

  public GetSmsFromArchive(ArchiveDemon archiveDaemon) {
    this.archiveDaemon = archiveDaemon;
  }

  public DataTableModel getSms(SmsQuery query, GetSmsProgress progress) throws AdminException {

    ru.novosoft.smsc.admin.archive_daemon.SmsQuery q = new ru.novosoft.smsc.admin.archive_daemon.SmsQuery();
    if (query.getAbonentAddress() != null)
      q.setAbonentAddress(query.getAbonentAddress().getSimpleAddress());
    if (query.getDstSmeId() != null)
      q.setDstSmeId(query.getDstSmeId());
    if (query.getFromAddress() != null)
      q.setFromAddress(query.getFromAddress().getSimpleAddress());
    if (query.getFromDate() != null)
      q.setFromDate(query.getFromDate());
    if (query.getLastResult() != null)
      q.setLastResult(query.getLastResult());
    if (query.getMaxRows() != null)
      q.setRowsMaximum(query.getMaxRows());
    if (query.getRouteId() != null)
      q.setRouteId(query.getRouteId());
    if (query.getSmeId() != null)
      q.setSmeId(query.getSmeId());
    if (query.getSmsId() != null)
      q.setSmsId(query.getSmsId().toString());
    if (query.getSrcSmeId() != null)
      q.setSrcSmeId(query.getSrcSmeId());
    if (query.getTillDate() != null)
      q.setTillDate(query.getTillDate());
    if (query.getToAddress() != null)
      q.setToAddress(query.getToAddress().getSimpleAddress());

    progress.setCurrent(0);
    progress.setTotal(1);
    final SmsSet messages = archiveDaemon.getSmsSet(q);
    progress.setCurrent(1);

    return new DataTableModel() {
      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
        List<Sms> result = new ArrayList<Sms>(count);

        for (int i = startPos; i < Math.min(messages.getRowsCount(), startPos + count); i++) {
          SmsRow r = messages.getRow(i);
          result.add(new SmsImpl(r));
        }

        return result;
      }

      public int getRowsCount() {
        return messages.getRowsCount();
      }
    };
  }

  private static class SmsImpl implements Sms {

    private final SmsRow r;

    public SmsImpl(SmsRow r) {
      this.r = r;
    }

    public long getId() throws AdminException {
      return r.getId();
    }

    public Date getSubmitTime() throws AdminException {
      return r.getSubmitTime();
    }

    public Date getValidTime() throws AdminException {
      return r.getValidTime();
    }

    public Integer getAttempts() throws AdminException {
      return r.getAttempts();
    }

    public Integer getLastResult() throws AdminException {
      return r.getLastResult();
    }

    public Date getLastTryTime() throws AdminException {
      return r.getLastTryTime();
    }

    public Date getNextTryTime() throws AdminException {
      return r.getNextTryTime();
    }

    public Address getOriginatingAddress() throws AdminException {
      return new Address(r.getOriginatingAddress());
    }

    public Address getDestinationAddress() throws AdminException {
      return new Address(r.getDestinationAddress());
    }

    public Address getDealiasedDestinationAddress() throws AdminException {
      return new Address(r.getDealiasedDestinationAddress());
    }

    public Integer getMessageReference() throws AdminException {
      return r.getMessageReference();
    }

    public String getServiceType() throws AdminException {
      return r.getServiceType();
    }

    public Integer getDeliveryReport() throws AdminException {
      return (int) r.getDeliveryReport();
    }

    public Integer getBillingRecord() throws AdminException {
      return (int) r.getBillingRecord();
    }

    public RoutingInfo getOriginatingDescriptor() throws AdminException {
      SmsDescriptor d = r.getOriginatingDescriptor();
      return new RoutingInfo(d.getImsi(), d.getMsc(), d.getSme());
    }

    public RoutingInfo getDestinationDescriptor() throws AdminException {
      SmsDescriptor d = r.getDestinationDescriptor();
      return new RoutingInfo(d.getImsi(), d.getMsc(), d.getSme());
    }

    public String getRouteId() throws AdminException {
      return r.getRouteId();
    }

    public Integer getServiceId() throws AdminException {
      return r.getServiceId();
    }

    public Integer getPriority() throws AdminException {
      return r.getPriority();
    }

    public String getSrcSmeId() throws AdminException {
      return r.getSrcSmeId();
    }

    public String getDstSmeId() throws AdminException {
      return r.getDstSmeId();
    }

    public byte getArc() throws AdminException {
      return r.getArc();
    }

    public byte[] getBody() throws AdminException {
      return r.getBody();
    }

    public String getOriginalText() throws AdminException {
      return r.getOriginalText();
    }

    public String getText() throws AdminException {
      return r.getText();
    }

    public boolean isTextEncoded() throws AdminException {
      return r.isTextEncoded();
    }
  }
}
