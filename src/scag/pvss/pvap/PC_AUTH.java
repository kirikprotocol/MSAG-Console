package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_AUTH {
  static Logger logger = Logger.getLogger(PC_AUTH.class);

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

  public PC_AUTH(int seqNum,byte protocolVersion, String login, String password, String name)
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
    if(protocolVersionFlag)
    {
      sb.append(";protocolVersion=");
      sb.append(protocolVersion.toString());    }
    if(loginFlag)
    {
      sb.append(";login=");
      sb.append(login.toString());    }
    if(passwordFlag)
    {
      sb.append(";password=");
      sb.append(password.toString());    }
    if(nameFlag)
    {
      sb.append(";name=");
      sb.append(name.toString());    }
    return sb.toString();
  }

  public int getSeqNum()
  {
    return seqNum;
  }

  public void setSeqNum(int seqNum)
  {
    this.seqNum = seqNum;
  }

  public byte getProtocolVersion()
  {
    if(!protocolVersionFlag)
    {
      //!!TODO!!
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
  {
    if(!loginFlag)
    {
      //!!TODO!!
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
  {
    if(!passwordFlag)
    {
      //!!TODO!!
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
  {
    if(!nameFlag)
    {
      //!!TODO!!
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

  public void encode(BufferWriter writer)
  {
    if(!protocolVersionFlag)
    {
      //!!TODO!!
    }
    if(!loginFlag)
    {
      //!!TODO!!
    }
    if(!passwordFlag)
    {
      //!!TODO!!
    }
    if(!nameFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)15); // tag id
    writer.writeByteLV(protocolVersion);
 
    writer.writeShort((short)16); // tag id
    writer.writeStringLV(login);
 
    writer.writeShort((short)17); // tag id
    writer.writeStringLV(password);
 
    writer.writeShort((short)18); // tag id
    writer.writeStringLV(name);
    writer.writeShort((short)0xFFFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException
  {
    seqNum = reader.readInt();
    while( true ) {
      int tag = reader.readShort();
      if( tag == (short)0xFFFF ) break;
      switch( tag ) {
        case 15:{
          protocolVersion = reader.readByteLV();          protocolVersionFlag=true;
          }break;
        case 16:{
          login = reader.readStringLV();          loginFlag=true;
          }break;
        case 17:{
          password = reader.readStringLV();          passwordFlag=true;
          }break;
        case 18:{
          name = reader.readStringLV();          nameFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_AUTH.class.getName());
      }

    }
  }
}
