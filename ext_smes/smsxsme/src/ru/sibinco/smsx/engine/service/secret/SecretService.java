package ru.sibinco.smsx.engine.service.secret;

import ru.sibinco.smsx.engine.service.ServiceController;
import ru.sibinco.smsx.engine.service.secret.commands.*;


/**
 * User: artem
 * Date: 29.06.2007
 */

public interface SecretService extends ServiceController, 
                                       SecretChangePasswordCmd.Receiver, SecretGetMessagesCmd.Receiver,
                                       SecretGetMessageStatusCmd.Receiver, SecretRegisterAbonentCmd.Receiver,
                                       SecretSendMessageCmd.Receiver, SecretUnregisterAbonentCmd.Receiver,
                                       SecretHandleReceiptCmd.Receiver {
}
