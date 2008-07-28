package ru.sibinco.smsx.engine.service.subscription;

import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsx.engine.service.subscription.commands.SubscriptionCheckCmd;

/**
 * User: artem
 * Date: 28.07.2008
 */

public interface SubscriptionService extends Service, SubscriptionCheckCmd.Receiver {
}
