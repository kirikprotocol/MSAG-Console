/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:57:16 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class ProfileAddCommand implements Command
{
    private final static String OPTION_MASK = "mask";
    private final static String OPTION_REP = "rep";
    private final static String OPTION_CP = "cp";

    public String process(String cmd) {
        return "ProfileAddCommand";
    }
}

