package ru.sibinco.smsx.engine.service.blacklist;

import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListAddCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListRemoveCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;

/**
 * User: artem
 * Date: 10.07.2007
 */

public interface BlackListService extends Service, BlackListAddCmd.Receiver, BlackListRemoveCmd.Receiver, BlackListCheckMsisdnCmd.Receiver{
}
