/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:24:53 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class ApplyCommand implements Command
{
    private Smsc smsc;

    public ApplyCommand(Smsc smsc) {
        this.smsc = smsc;
    }

    public String process(String cmd)
    {
        String output = "";
        try {
            //smsc.applyProfiles();
            smsc.applyAliases();
            smsc.applyRoutes();
            output = "Changes applied succesfully";
        } catch (Exception e) {
            output = "Failed to apply changes";
            //logger.error(output, e);
            e.printStackTrace();
        }
        return output;
    }
}

