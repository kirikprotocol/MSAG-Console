/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:54:05 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class AliasEditCommand implements Command
{
    private final static String OPTION_ADDRESS = "address";
    private final static String OPTION_ALIAS = "alias";
    private final static String OPTION_HIDE = "hide";

    public String process(String cmd) {
        return "AliasEditCommand";
    }
}

