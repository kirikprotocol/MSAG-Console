package ru.novosoft.smsc.web.controllers.sms_view;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
interface GetSmsStrategy {

  DataTableModel getSms(SmsQuery query, GetSmsProgress progress) throws AdminException;
}
