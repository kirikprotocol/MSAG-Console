package mobi.eyeline.informer.admin.delivery.protogen.protocol;

import mobi.eyeline.protogen.framework.*;
import java.io.IOException;



/**
 * This class was generated by protogen, do not edit it
 */
public class MessageFlags  {
 
  boolean transactionMode;
  boolean transactionModeFlag=false;
  boolean useDataSm;
  boolean useDataSmFlag=false;
  boolean replaceMessage;
  boolean replaceMessageFlag=false;
  boolean flash;
  boolean flashFlag=false;
  String svcType;
  boolean svcTypeFlag=false;
  String sourceAddress;
  boolean sourceAddressFlag=false;
  DeliveryMode deliveryMode;
  boolean deliveryModeFlag=false;

  public MessageFlags() {
  }
 

  public MessageFlags(boolean transactionMode , boolean useDataSm , boolean replaceMessage , boolean flash , String svcType , String sourceAddress , DeliveryMode deliveryMode) {
    this.transactionMode = transactionMode;
    this.transactionModeFlag = true;
    this.useDataSm = useDataSm;
    this.useDataSmFlag = true;
    this.replaceMessage = replaceMessage;
    this.replaceMessageFlag = true;
    this.flash = flash;
    this.flashFlag = true;
    this.svcType = svcType;
    this.svcTypeFlag = true;
    this.sourceAddress = sourceAddress;
    this.sourceAddressFlag = true;
    this.deliveryMode = deliveryMode;
    this.deliveryModeFlag = true;
  }
 

  public void clear() {
    transactionModeFlag=false;
    useDataSmFlag=false;
    replaceMessageFlag=false;
    flashFlag=false;
    svcTypeFlag=false;
    sourceAddressFlag=false;
    deliveryModeFlag=false;
  }
 
  public void debug(StringBuilder sb) {

    if(transactionModeFlag) { sb.append("transactionMode=") .append(transactionMode).append(';'); }

    if(useDataSmFlag) { sb.append("useDataSm=") .append(useDataSm).append(';'); }

    if(replaceMessageFlag) { sb.append("replaceMessage=") .append(replaceMessage).append(';'); }

    if(flashFlag) { sb.append("flash=") .append(flash).append(';'); }

    if(svcTypeFlag) { sb.append("svcType=") .append(svcType).append(';'); }

    if(sourceAddressFlag) { sb.append("sourceAddress=") .append(sourceAddress).append(';'); }

    if(deliveryModeFlag) { sb.append("deliveryMode=") .append(deliveryMode); }
  }
 
  public String toString() {
    StringBuilder sb=new StringBuilder(128);
    sb.append("MessageFlags:");
    debug(sb);
    return sb.toString();
  }


  public boolean getTransactionMode() throws AccessToUndefinedFieldException {
    if(!transactionModeFlag) throw new AccessToUndefinedFieldException("transactionMode");
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
    this.transactionModeFlag = true;
  }
 
  public boolean hasTransactionMode() {
    return transactionModeFlag;
  }
  public boolean getUseDataSm() throws AccessToUndefinedFieldException {
    if(!useDataSmFlag) throw new AccessToUndefinedFieldException("useDataSm");
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
    this.useDataSmFlag = true;
  }
 
  public boolean hasUseDataSm() {
    return useDataSmFlag;
  }
  public boolean getReplaceMessage() throws AccessToUndefinedFieldException {
    if(!replaceMessageFlag) throw new AccessToUndefinedFieldException("replaceMessage");
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage) {
    this.replaceMessage = replaceMessage;
    this.replaceMessageFlag = true;
  }
 
  public boolean hasReplaceMessage() {
    return replaceMessageFlag;
  }
  public boolean getFlash() throws AccessToUndefinedFieldException {
    if(!flashFlag) throw new AccessToUndefinedFieldException("flash");
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
    this.flashFlag = true;
  }
 
  public boolean hasFlash() {
    return flashFlag;
  }
  public String getSvcType() throws AccessToUndefinedFieldException {
    if(!svcTypeFlag) throw new AccessToUndefinedFieldException("svcType");
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
    this.svcTypeFlag = true;
  }
 
  public boolean hasSvcType() {
    return svcTypeFlag;
  }
  public String getSourceAddress() throws AccessToUndefinedFieldException {
    if(!sourceAddressFlag) throw new AccessToUndefinedFieldException("sourceAddress");
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
    this.sourceAddressFlag = true;
  }
 
  public boolean hasSourceAddress() {
    return sourceAddressFlag;
  }
  public DeliveryMode getDeliveryMode() throws AccessToUndefinedFieldException {
    if(!deliveryModeFlag) throw new AccessToUndefinedFieldException("deliveryMode");
    return deliveryMode;
  }

  public void setDeliveryMode(DeliveryMode deliveryMode) {
    this.deliveryMode = deliveryMode;
    this.deliveryModeFlag = true;
  }
 
  public boolean hasDeliveryMode() {
    return deliveryModeFlag;
  }

  public void encode(BufferWriter writer) throws IOException {
    if(transactionModeFlag) {
      writer.writeTag(1); // tag id
      writer.writeBoolLV(transactionMode);
    }  else throw new MissingMandatoryFieldException("transactionMode");
    if(useDataSmFlag) {
      writer.writeTag(2); // tag id
      writer.writeBoolLV(useDataSm);
    }  else throw new MissingMandatoryFieldException("useDataSm");
    if(replaceMessageFlag) {
      writer.writeTag(3); // tag id
      writer.writeBoolLV(replaceMessage);
    }  else throw new MissingMandatoryFieldException("replaceMessage");
    if(flashFlag) {
      writer.writeTag(4); // tag id
      writer.writeBoolLV(flash);
    }  else throw new MissingMandatoryFieldException("flash");
    if(svcTypeFlag) {
      writer.writeTag(5); // tag id
      writer.writeStringLV(svcType);
    } 
    if(sourceAddressFlag) {
      writer.writeTag(6); // tag id
      writer.writeStringLV(sourceAddress);
    } 
    if(deliveryModeFlag) {
      writer.writeTag(7); // tag id
      writer.writeByteLV(deliveryMode.getValue());
    } 
    writer.writeTag(0xFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException {
    while( true ) {
      int tag = reader.readTag();
      if( tag == 0xFF ) break;
      switch( tag ) {
        case 1: {
          transactionMode = reader.readBoolLV();
          transactionModeFlag=true;
        } break;
        case 2: {
          useDataSm = reader.readBoolLV();
          useDataSmFlag=true;
        } break;
        case 3: {
          replaceMessage = reader.readBoolLV();
          replaceMessageFlag=true;
        } break;
        case 4: {
          flash = reader.readBoolLV();
          flashFlag=true;
        } break;
        case 5: {
          svcType = reader.readStringLV();
          svcTypeFlag=true;
        } break;
        case 6: {
          sourceAddress = reader.readStringLV();
          sourceAddressFlag=true;
        } break;
        case 7: {
          deliveryMode = DeliveryMode.valueOf(reader.readByteLV());
          deliveryModeFlag=true;
        } break;
        default:
          throw new IOException( "unknown tagId: "+tag +" msg: "+MessageFlags.class.getName());      }
    }
  }
}
