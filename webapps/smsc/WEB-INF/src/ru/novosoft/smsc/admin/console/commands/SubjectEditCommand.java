/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:05:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class SubjectEditCommand implements Command
{
    private final static String OPTION_SUBJECT = "subject";
    private final static String OPTION_ADDMASKS = "addmasks";
    private final static String OPTION_MASKS = "masks";

    public String process(String cmd) {
        return "SubjectEditCommand";
    }
}

