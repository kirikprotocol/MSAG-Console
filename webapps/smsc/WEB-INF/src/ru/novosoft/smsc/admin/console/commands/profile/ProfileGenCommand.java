/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 23, 2002
 * Time: 2:33:18 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.profile;

import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.profiler.Profile;

import java.util.Iterator;

import antlr.RecognitionException;

public abstract class ProfileGenCommand extends CommandClass {
    public final static byte ALIAS_HIDE = Profile.ALIAS_HIDE_true;
    public final static byte ALIAS_NOHIDE = Profile.ALIAS_HIDE_false;
    public final static byte ALIAS_SUBSTIT = Profile.ALIAS_HIDE_substitute;

    protected byte report = Profile.REPORT_OPTION_None;
    protected byte codepage = Profile.CODEPAGE_Default;
    protected boolean ussd7bit = false;

    protected byte aliasHide = ALIAS_HIDE;
    protected boolean aliasModifiable = false;

    protected String divert = "";
    protected boolean divertActiveOn = false;
    protected boolean divertActiveAbsent = false;
    protected boolean divertActiveBarred = false;
    protected boolean divertActiveBlocked = false;
    protected boolean divertActiveCapacity = false;
    protected boolean divertActiveUnconditional = false;
    protected boolean divertModifiable = false;
    protected boolean udhConcat = true;
    protected boolean translit = true;
    protected int groupId = 0;
    protected String groupIdOrName = "";
    protected int inputAccessMask = 1;
    protected int outputAccessMask = 1;
    protected int services = 0;

    protected boolean isAliasOptions = false;
    protected boolean isDivertOptions = false;
    protected boolean isReport = false;
    protected boolean isCodepage = false;
    protected boolean isAliasHide = false;
    protected boolean isAliasModifiable = false;
    protected boolean isDivert = false;
    protected boolean isDivertActiveAbsent = false;
    protected boolean isDivertActiveBarred = false;
    protected boolean isDivertActiveBlocked = false;
    protected boolean isDivertActiveCapacity = false;
    protected boolean isDivertActiveUnconditional = false;
    protected boolean isDivertModifiable = false;
    protected boolean isUdhConcat = false;
    protected boolean isTranslit = false;
    protected boolean isInputAccessMask = false;
    protected boolean isOutputAccessMask = false;
    protected boolean isGroupNameOrId = false;
    protected boolean isServices = false;

    protected String locale;
    protected boolean isLocale = false;

    public void process(CommandContext ctx) {
        if (isGroupNameOrId)
            for (Iterator i = ctx.getClosedGroupManager().getClosedGroups().iterator(); i.hasNext();) {
                ClosedGroup cg = (ClosedGroup) i.next();
                if (Integer.toString(cg.getId()).equals(groupIdOrName) || (cg.getName().equals(groupIdOrName))) {
                    groupId = cg.getId();
                    break;
                }
            }
    }

    public void setNoneReport() {
        this.report = Profile.REPORT_OPTION_None;
        isReport = true;
    }

    public void setFullReport() {
        this.report = Profile.REPORT_OPTION_Full;
        isReport = true;
    }

    public void setFinalReport() {
        this.report = Profile.REPORT_OPTION_Final;
        isReport = true;
    }

    public void setLocale(String locale) {
        this.locale = locale;
        isLocale = true;
    }

    public void setGsm7Encoding() {
        this.codepage = Profile.CODEPAGE_Default;
        isCodepage = true;
    }

    public void setUcs2Encoding() {
        this.codepage = Profile.CODEPAGE_UCS2;
        isCodepage = true;
    }

    public void setLatin1Encoding() {
        this.codepage = Profile.CODEPAGE_Latin1;
        isCodepage = true;
    }

    public void setUcs2Latin1Encoding() {
        this.codepage = Profile.CODEPAGE_UCS2AndLatin1;
        isCodepage = true;
    }

    public void setUssd7Bit(boolean ussd7bit) {
        this.ussd7bit = ussd7bit;
    }

    public void setAliasOptions(boolean aliasOptions) {
        isAliasOptions = aliasOptions;
    }

    public void setAliasHide(byte aliasHide) {
        this.aliasHide = aliasHide;
        isAliasHide = true;
    }

    public void setAliasModifiable(boolean aliasModifiable) {
        this.aliasModifiable = aliasModifiable;
        isAliasModifiable = true;
    }

    public void setDivertOptions(boolean divertOptions) {
        isDivertOptions = divertOptions;
    }

    public void setDivert(String divert) {
        this.divert = divert;
        isDivert = true;
        isDivertOptions = true;
    }

    public void setDivertActiveOn(boolean on) {
        this.divertActiveOn = on;
        isDivertOptions = true;
    }

    public void setDivertActiveAbsent(boolean divertActiveAbsent) {
        this.divertActiveAbsent = divertActiveAbsent;
        this.isDivertActiveAbsent = true;
    }

    public void setDivertActiveBarred(boolean divertActiveBarred) {
        this.divertActiveBarred = divertActiveBarred;
        this.isDivertActiveBarred = true;
    }

    public void setDivertActiveBlocked(boolean divertActiveBlocked) {
        this.divertActiveBlocked = divertActiveBlocked;
        this.isDivertActiveBlocked = true;
    }

    public void setDivertActiveCapacity(boolean divertActiveCapacity) {
        this.divertActiveCapacity = divertActiveCapacity;
        this.isDivertActiveCapacity = true;
    }

    public void setDivertActiveUnconditional(boolean divertActiveUnconditional) {
        this.divertActiveUnconditional = divertActiveUnconditional;
        this.isDivertActiveUnconditional = true;
    }

    public void setDivertModifiable(boolean divertModifiable) {
        this.divertModifiable = divertModifiable;
        isDivertModifiable = true;
    }

    public void setUdhConcat(boolean udhConcat) {
        this.udhConcat = udhConcat;
        isUdhConcat = true;
    }

    public void setTranslit(boolean translit) {
        this.translit = translit;
        isTranslit = true;
    }

    public void setInputAccessMask(int inputAccessMask) {
        this.inputAccessMask = inputAccessMask;
        this.isInputAccessMask = true;
    }

    public void setInputAccessBit(boolean value, int bitNum) throws RecognitionException {
        if (bitNum < 0 && bitNum > 31) throw new RecognitionException("incorrect bit value");
        if (value) inputAccessMask = inputAccessMask | (1 << bitNum);
              else inputAccessMask = inputAccessMask & ~(1 << bitNum);
        this.isInputAccessMask = true;
    }

    public void setOutputAccessMask(int outputAccessMask) {
        this.outputAccessMask = outputAccessMask;
        this.isOutputAccessMask = true;
    }

    public void setOutputAccessBit(boolean value, int bitNum) throws RecognitionException {
        if (bitNum < 0 && bitNum > 31) throw new RecognitionException("incorrect bit value");
        if (value) outputAccessMask = outputAccessMask | (1 << bitNum);
              else outputAccessMask = outputAccessMask & ~(1 << bitNum);
        this.isOutputAccessMask = true;
    }

    public void setGroupIdOrName(String group) {
        this.groupIdOrName = group;
        this.isGroupNameOrId = true;
    }

    public void setServices(int services) {
        this.services = services;
        this.isServices = true;
    }

    public void setServicesBit(boolean value, int bitNum) throws RecognitionException {
        if (bitNum < 0 && bitNum > 31) throw new RecognitionException("incorrect bit value");
        if (value) services = services | (1 << bitNum);
              else services = services & ~(1 << bitNum);
        this.isServices = true;
    }
}
