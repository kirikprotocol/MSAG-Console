/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:49:04 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class AliasDeleteCommand implements Command
{
    private final static String OPTION_ALIAS = "alias";

    public String process(String cmd) {
        return "AliasDeleteCommand";
    }
}

