package ru.sibinco.smsx.engine.service.sponsored;

import ru.sibinco.smsx.engine.service.ServiceController;
import ru.sibinco.smsx.engine.service.sponsored.commands.SponsoredRegisterAbonentCmd;
import ru.sibinco.smsx.engine.service.sponsored.commands.SponsoredUnregisterAbonentCmd;
import ru.sibinco.smsx.engine.service.sponsored.commands.SponsoredRegisterDeliveryCmd;

/**
 * User: artem
 * Date: 29.06.2007
 */

public interface SponsoredService extends ServiceController, SponsoredRegisterAbonentCmd.Receiver, SponsoredUnregisterAbonentCmd.Receiver,
                                    SponsoredRegisterDeliveryCmd.Receiver {

}
