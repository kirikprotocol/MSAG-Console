package ru.sibinco.smsx.engine.service.nick;

import ru.sibinco.smsx.engine.service.nick.commands.NickSendMessageCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickRegisterCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickUnregisterCmd;
import ru.sibinco.smsx.engine.service.Service;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public interface NickService extends Service, NickRegisterCmd.Receiver, NickSendMessageCmd.Receiver, NickUnregisterCmd.Receiver {
}
