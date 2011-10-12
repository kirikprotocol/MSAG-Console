package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveMessageFilter;

/**
 * @author Artem Snopkov
 */
interface GetSmsStrategy {

  DataTableModel getSms(ArchiveMessageFilter query, GetSmsProgress progress) throws AdminException;
}
