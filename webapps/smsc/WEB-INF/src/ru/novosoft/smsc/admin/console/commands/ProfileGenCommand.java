/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 23, 2002
 * Time: 2:33:18 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.profiler.Profile;

public abstract class ProfileGenCommand implements Command
{
    protected byte report = Profile.REPORT_OPTION_None;
    protected byte codepage = Profile.CODEPAGE_Default;

    public void setNoneReport() {
        this.report = Profile.REPORT_OPTION_None;
    }
    public void setFullReport() {
        this.report = Profile.REPORT_OPTION_Full;
    }
    public void setGsm7Encoding() {
        this.codepage = Profile.CODEPAGE_Default;
    }
    public void setUcs2Encoding() {
        this.codepage = Profile.CODEPAGE_UCS2;
    }
}
