template <class MSG>
communication::libsua_messages::N_STATE_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_STATE_Indication_message(const MSG& message, uint8_t userStatus)
{
  communication::libsua_messages::N_STATE_IND_Message resultMesage;

  communication::sua_messages::ITU_PC affectedPointCode;
  if ( message.getAffectedPointCode().getNextPC(&affectedPointCode) ) {
    uint16_t pointCode;
    memcpy(reinterpret_cast<uint8_t*>(&pointCode), affectedPointCode.getValue()+2, sizeof(pointCode));
    pointCode = ntohs(pointCode);
    resultMesage.setPointCode(pointCode);

    resultMesage.setSSN(uint8_t(message.getSSN().getValue()));
    resultMesage.setUserStatus(userStatus);
    if ( message.getSMI().isSetValue() )
      resultMesage.setSubsystemMultiplicityInd(uint8_t(message.getSMI().getValue()));

    return resultMesage;
  } else
    throw smsc::util::Exception("SignalingNetworkManagementMessageHandlers::make_N_STATE_Indication_message::: affected point code is absent in original %s message", message.getMsgCodeTextDescription());
}
