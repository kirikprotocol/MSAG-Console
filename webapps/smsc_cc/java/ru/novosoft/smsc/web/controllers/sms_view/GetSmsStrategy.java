package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;

/**
 * @author Artem Snopkov
 */
interface GetSmsStrategy {

  DataTableModel getSms(SmsQuery query, GetSmsProgress progress) throws AdminException;
}
