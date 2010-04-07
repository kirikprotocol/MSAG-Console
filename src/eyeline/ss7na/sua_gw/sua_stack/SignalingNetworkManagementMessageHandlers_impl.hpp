template <class MSG>
libsccp::N_STATE_IND_Message
SignalingNetworkManagementMessageHandlers::make_N_STATE_Indication_message(const MSG& message,
                                                                           uint8_t userStatus)
{
  libsccp::N_STATE_IND_Message resultMesage;

  common::ITU_PC affectedPointCode;
  size_t offset=0;
  if ( message.getAffectedPointCode().getNextPC(&affectedPointCode, &offset) ) {
    resultMesage.setPointCode(affectedPointCode.getValue());

    resultMesage.setSSN(uint8_t(message.getSSN().getValue()));
    resultMesage.setUserStatus(userStatus);
    if ( message.getSMI().isSetValue() )
      resultMesage.setSubsystemMultiplicityInd(uint8_t(message.getSMI().getValue()));

    return resultMesage;
  } else
    throw smsc::util::Exception("SignalingNetworkManagementMessageHandlers::make_N_STATE_Indication_message::: affected point code is absent in original %s message", message.getMsgCodeTextDescription());
}
