package ru.novosoft.smsc.admin.protocol;

public class CommandSwitchOverService extends ServiceCommand {
    public CommandSwitchOverService(String serviceName) {
        super("switchover_service", serviceName);
    }
}
