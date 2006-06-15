package ru.novosoft.smsc.admin.console.commands.emailsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.console.CommandContext;
import antlr.RecognitionException;

public class EmailSmeAddCommand extends EmailSmeGenCommand {
    private String forwardEmail = "";
    private String userName = "";
    private String realName = "";
    private byte limitType;
    private int limitValue;

    private boolean isForwardEmail = false;
    private boolean isRealName = false;

    public void process(CommandContext ctx) {
        String out = "emailsme '" + address + "'";
        try {
            EmailSmeContext context = EmailSmeContext.getInstance(ctx.getOwner().getContext());
            context.add(ton, npi, address, userName, forwardEmail, realName, limitType, 0, limitValue, 0, 0);
            ctx.setMessage(out + " added or updated.");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (AdminException e) {
            ctx.setMessage("Couldn't add " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "EMAILSME_ADD";
    }

    public void setForwardEmail(String forwardEmail) {
        this.forwardEmail = forwardEmail;
        isForwardEmail = true;
    }

    public void setRealName(String realName) {
        this.realName = realName;
        isRealName = true;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }

    public void setLimitType(String type) throws RecognitionException{
        if (type.equals(EmailSmeContext.LIMITCHAR_DAY_CHAR)) {limitType = EmailSmeContext.LIMITTYPE_DAY; return;}
        else if (type.equals(EmailSmeContext.LIMITCHAR_DAY_WEEK)) {limitType = EmailSmeContext.LIMITTYPE_WEEK; return;}
        else if (type.equals(EmailSmeContext.LIMITCHAR_DAY_MONTH)) {limitType = EmailSmeContext.LIMITTYPE_MONTH; return;}
        else throw new RecognitionException("incorrect type value");
    }

    public void setLimitValue(int value) {
        this.limitValue = value;
    }
}
