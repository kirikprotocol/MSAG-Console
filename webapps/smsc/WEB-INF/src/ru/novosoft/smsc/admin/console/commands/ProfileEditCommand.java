/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:55:34 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class ProfileEditCommand implements Command
{
    private final static String OPTION_ADDRESS = "address";
    private final static String OPTION_CP = "cp";
    private final static String OPTION_REP = "rep";

    public String process(String cmd) {
        return "ProfileEditCommand";
    }
}

