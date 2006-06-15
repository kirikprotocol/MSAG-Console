package ru.novosoft.smsc.admin.console.commands.emailsme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;

public abstract class EmailSmeGenCommand extends CommandClass {
    protected byte ton;
    protected byte npi;
    protected String address = "";

    public void setTon(int ton) {
        this.ton = (byte) ton;
    }

    public void setNpi(int npi) {
        this.npi = (byte) npi;
    }

    public void setAddress(String address) {
        this.address = address;
    }
}
