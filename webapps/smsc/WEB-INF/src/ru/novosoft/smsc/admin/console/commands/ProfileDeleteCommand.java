/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class ProfileDeleteCommand implements Command
{
    private final static String OPTION_MASK = "mask";

    public String process(String cmd) {
        return "ProfileDeleteCommand";
    }
}

