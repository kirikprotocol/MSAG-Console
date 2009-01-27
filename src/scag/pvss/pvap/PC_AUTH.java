package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_AUTH  
{
    // static Logger logger = Logger.getLogger(PC_AUTH.class);

    static final short protocolVersionTag = 15;
    static final short loginTag = 16;
    static final short passwordTag = 17;
    static final short nameTag = 18;

    int seqNum;
    byte protocolVersion;
    boolean protocolVersionFlag=false;
    String login;
    boolean loginFlag=false;
    String password;
    boolean passwordFlag=false;
    String name;
    boolean nameFlag=false;

    public PC_AUTH() {
    }

    public PC_AUTH(int seqNum, byte protocolVersion , String login , String password , String name)
    {
        this.seqNum = seqNum;
        this.protocolVersion = protocolVersion;
        this.protocolVersionFlag = true;
        this.login = login;
        this.loginFlag = true;
        this.password = password;
        this.passwordFlag = true;
        this.name = name;
        this.nameFlag = true;
    }
 
    public void clear()
    {
        protocolVersionFlag=false;
        loginFlag=false;
        passwordFlag=false;
        nameFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_AUTH:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (protocolVersionFlag) {
            sb.append(";protocolVersion=");
            sb.append(protocolVersion);
        }
        if (loginFlag) {
            sb.append(";login=");
            sb.append(login);
        }
        if (passwordFlag) {
            sb.append(";password=");
            sb.append(password);
        }
        if (nameFlag) {
            sb.append(";name=");
            sb.append(name);
        }
        return sb.toString();
    }

    public byte getProtocolVersion() throws FieldIsNullException
    {
        if(!protocolVersionFlag)
        {
            throw new FieldIsNullException("protocolVersion");
        }
        return protocolVersion;
    }

    public void setProtocolVersion(byte protocolVersion)
    {
        this.protocolVersion = protocolVersion;
        this.protocolVersionFlag = true;
    }

    public boolean hasProtocolVersion()
    {
        return protocolVersionFlag;
    }

    public String getLogin() throws FieldIsNullException
    {
        if(!loginFlag)
        {
            throw new FieldIsNullException("login");
        }
        return login;
    }

    public void setLogin(String login)
    {
        this.login = login;
        this.loginFlag = true;
    }

    public boolean hasLogin()
    {
        return loginFlag;
    }

    public String getPassword() throws FieldIsNullException
    {
        if(!passwordFlag)
        {
            throw new FieldIsNullException("password");
        }
        return password;
    }

    public void setPassword(String password)
    {
        this.password = password;
        this.passwordFlag = true;
    }

    public boolean hasPassword()
    {
        return passwordFlag;
    }

    public String getName() throws FieldIsNullException
    {
        if(!nameFlag)
        {
            throw new FieldIsNullException("name");
        }
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
        this.nameFlag = true;
    }

    public boolean hasName()
    {
        return nameFlag;
    }

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(protocolVersionTag);
        writer.writeByteLV(protocolVersion);
        writer.writeTag(loginTag);
        writer.writeStringLV(login);
        writer.writeTag(passwordTag);
        writer.writeStringLV(password);
        writer.writeTag(nameTag);
        writer.writeStringLV(name);
        // optional fields
    }

    public void decode( IBufferReader reader ) throws java.io.IOException
    {
        clear();
        // seqNum = reader.readInt();
        while( true ) {
            short tag = reader.readTag();
            // System.out.println("tag got:" + tag);
            if ( tag == (short)0xFFFF ) break;
            switch( tag ) {
            case protocolVersionTag: {
                protocolVersion=reader.readByteLV();
                protocolVersionFlag=true;
                break;
            }
            case loginTag: {
                login=reader.readStringLV();
                loginFlag=true;
                break;
            }
            case passwordTag: {
                password=reader.readStringLV();
                passwordFlag=true;
                break;
            }
            case nameTag: {
                name=reader.readStringLV();
                nameFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_AUTH.class.getName() );
            }
        }
        checkFields();
    }

    public int getSeqNum()
    {
        return seqNum;
    }

    public void setSeqNum(int seqNum)
    {
        this.seqNum = seqNum;
    }

    protected void checkFields() throws MandatoryFieldMissingException
    {
        // checking mandatory fields
        if (!protocolVersionFlag) {
            throw new MandatoryFieldMissingException("protocolVersion");
        }
        if (!loginFlag) {
            throw new MandatoryFieldMissingException("login");
        }
        if (!passwordFlag) {
            throw new MandatoryFieldMissingException("password");
        }
        if (!nameFlag) {
            throw new MandatoryFieldMissingException("name");
        }
        // checking optional fields
    }
}
