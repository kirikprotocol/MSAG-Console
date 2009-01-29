package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_AUTH  
{
    // static Logger logger = Logger.getLogger(PC_AUTH.class);

    static final int protocolVersionTag = 15;
    static final int loginTag = 16;
    static final int passwordTag = 17;
    static final int nameTag = 18;

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

    public byte getProtocolVersion()
           throws FieldIsNullException
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

    public String getLogin()
           throws FieldIsNullException
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

    public String getPassword()
           throws FieldIsNullException
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

    public String getName()
           throws FieldIsNullException
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

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + protocolVersionTag);
        writer.writeTag(protocolVersionTag);
        writer.writeByteLV(protocolVersion);
        System.out.println("write pos=" + writer.getPos() + " field=" + loginTag);
        writer.writeTag(loginTag);
        writer.writeUTFLV(login);
        System.out.println("write pos=" + writer.getPos() + " field=" + passwordTag);
        writer.writeTag(passwordTag);
        writer.writeUTFLV(password);
        System.out.println("write pos=" + writer.getPos() + " field=" + nameTag);
        writer.writeTag(nameTag);
        writer.writeUTFLV(name);
        // optional fields
    }

    public void decode( PVAP proto, IBufferReader reader ) throws java.io.IOException
    {
        clear();
        while( true ) {
            int pos = reader.getPos();
            int tag = reader.readTag();
            System.out.println("read pos=" + pos + " field=" + tag);
            if ( tag == -1 ) break;
            switch( tag ) {
            case protocolVersionTag: {
                if (protocolVersionFlag) {
                    throw new DuplicateFieldException("protocolVersion");
                }
                protocolVersion=reader.readByteLV();
                protocolVersionFlag=true;
                break;
            }
            case loginTag: {
                if (loginFlag) {
                    throw new DuplicateFieldException("login");
                }
                login=reader.readUTFLV();
                loginFlag=true;
                break;
            }
            case passwordTag: {
                if (passwordFlag) {
                    throw new DuplicateFieldException("password");
                }
                password=reader.readUTFLV();
                passwordFlag=true;
                break;
            }
            case nameTag: {
                if (nameFlag) {
                    throw new DuplicateFieldException("name");
                }
                name=reader.readUTFLV();
                nameFlag=true;
                break;
            }
            default:
                throw new NotImplementedException("reaction of reading unknown");
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
