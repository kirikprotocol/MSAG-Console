/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:00:32 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class SubjectDeleteCommand implements Command
{
    private final static String OPTION_SUBJECT = "subject";

    public String process(String cmd) {
        return "SubjectDeleteCommand";
    }
}

