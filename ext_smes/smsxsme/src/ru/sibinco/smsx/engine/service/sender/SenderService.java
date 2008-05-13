package ru.sibinco.smsx.engine.service.sender;

import ru.sibinco.smsx.engine.service.ServiceController;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;

/**
 * User: artem
 * Date: 06.07.2007
 */

public interface SenderService extends ServiceController, SenderGetMessageStatusCmd.Receiver, SenderSendMessageCmd.Receiver,
                                       SenderHandleReceiptCmd.Receiver {
}
