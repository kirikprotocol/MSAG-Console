#include "MmsContext.h"

namespace scag { namespace transport { namespace mms {

ActionID MmsContext::actionNext[8] = {
  SEND_REQUEST,               // <- PROCESS_REQUEST
  SEND_RESPONSE,              // <- PROCESS_RESPONSE
  NOP,                        // <- PROCESS_STATUS_RESPONSE
  PROCESS_REQUEST,            // <- READ_REQUEST
  READ_RESPONSE,              // <- SEND_REQUEST
  PROCESS_RESPONSE,           // <- READ_RESPONSE
  PROCESS_STATUS_RESPONSE     // <- SEND_RESPONSE
};

const char *MmsContext::taskName[8] = {
  "Scag",             // <- PROCESS_REQUEST
  "Scag",             // <- PROCESS_RESPONSE 
  "Scag",             // <- PROCESS_STATUS_RESPONSE
  "Reader",           // <- READ_REQUEST
  "Writer",           // <- SEND_REQUEST
  "Reader",           // <- READ_RESPONSE
  "Writer"            // <- SEND_RESPONSE
};

void MmsContext::setTransactionId(unsigned int tid) {
  transaction_id.clear();
  const size_t buf_size = 12;
  char buf[buf_size];
  int n = snprintf(buf, buf_size, "%d", tid);
  transaction_id.append(buf, n);
}

void MmsContext::replaceTid() {
  string tid = command->getTransactionId();
  command->setTransactionId(transaction_id);
  transaction_id = tid;
}

bool MmsContext::createResponse(Socket* s) {
  MmsCommand *cmd = new MmsResponse();
  if (cmd->createMM7Command(http_packet.getSoapEnvelope(),
                             http_packet.getSoapEnvelopeSize())) {
    result = 0;
  } else {
    result = status::SERVICE_UNAVAILABLE; //status???
    flags = FAKE_RESP | DEL_SERVICE_SOCK;
    delete cmd;
    return false;
  }
  s->Abort();
  service = NULL;
  delete s;
  if (command) {
    delete command;
  }
  command = cmd;
  action = PROCESS_RESPONSE;
  return true;
}

bool MmsContext::createRequest() {
  if (command) {
    delete command;
  }
  command = new MmsRequest();
  if (command->createMM7Command(http_packet.getSoapEnvelope(),
                                 http_packet.getSoapEnvelopeSize())) {
    action = PROCESS_REQUEST;
  } else {
    result = status::VALIDATION_ERROR;
    flags = FAKE_RESP;
    action = PROCESS_RESPONSE;
    return false;
  }
  result = 0;
  return true;
}

void MmsContext::createFakeResponse() {
  if (result && result < status::SUCSESS) {
    http_packet.createFakeResp(result);
    return;
  }
  if (!result || result < status::CLIENT_ERROR) {
    return;
  }
  if (!command) {
    result = http_status::INTERNAL_SERVER_ERROR;
    http_packet.createFakeResp(result);
    return;
  }

  MmsCommand *cmd = new MmsResponse();
  replaceTid();
  if (!cmd->createResponse(command->getMmsMsg(), result)) {
    if (command->isRequest()) {
      if (command->getTransactionId().empty()) {
        result = http_status::INTERNAL_SERVER_ERROR;
        http_packet.createFakeResp(result);
        return;
      }
      transaction_id = command->getTransactionId();
    }
    cmd->createGenericError(transaction_id, result, is_vasp);
  }
  delete command;
  command = cmd;
  http_packet.clear();
  string soap_envelope;
  command->serialize(soap_envelope);
  http_packet.setSoapEnvelope(soap_envelope);
  http_packet.fillResponse();
}

void MmsContext::serializeRequest() {
  string soap_envelope;
  if (!command || !command->serialize(soap_envelope)) {
    result = http_status::INTERNAL_SERVER_ERROR;
    createFakeResponse();
    return;
  }
  http_packet.setSoapEnvelope(soap_envelope);
}

void MmsContext::serializeResponse() {
  if (result) {
    return;
  }
  string soap_envelope;
  if (!command || !command->serialize(soap_envelope)) {
    result = status::SERVICE_ERROR;
    createFakeResponse();
    return;
  }
  http_packet.setSoapEnvelope(soap_envelope);
}

MmsContext::~MmsContext()
{
  if (client) {
    delete client;
  }
  if (service) {
    delete service;
  }
  if (command) {
    delete command;
  }
}

/*
bool MmsContext::createCommand(Socket* s) {
  std::string soap_envelope = http_packet.getSoapEnvelope();
  if (action = READ_RESPONSE) {
    MmsCommand *cmd = new MmsResponse();
    if (cmd->createMM7Command(soap_envelope.c_str(), soap_envelope.size())) {
      result = 0;
    } else {
      //Internal Server Error
      if (!command || !cmd->createResponse(command->getMmsMsg(), "4005", "Service Error")) {
        if (is_vasp) {
          cmd->createVASPError(transaction_id, "4005", "Service Error");
        } else {
          cmd->createRSError(transaction_id, "4005", "Service Error");
        }
      }
      //result = 503;
    }
    s->Abort();
    service = NULL;
    delete s;
    if (command) {
      delete command;
    }
    command = cmd;
    action = PROCESS_RESPONSE;
  } else {
    if (command) {
      delete command;
    }
    command = new MmsRequest();
    if (command->createMM7Command(soap_envelope.c_str(), soap_envelope.size())) {
      action = PROCESS_REQUEST;
    } else {
      if (!command->createResponse("4004", "Validation Error")) {
        if (command->getTransactionId().empty()) {
          result = 400;
          return false;
        } else {
          if (is_vasp) {
            command->createVASPError(command->getTransactionId(), "4004", "Validation Error");
          } else {
            command->createRSError(command->getTransactionId(), "4004", "Validation Error");
          }
        }
      }
      action = PROCESS_RESPONSE;
    }
  }
  return true;
}

/*
bool MmsContext::createCommand(Socket* s) {
  std::string soap_envelope = http_packet.getSoapEnvelope();
  if (command.createMM7Command(soap_envelope.c_str(), soap_envelope.size())) {
    if (action == READ_RESPONSE) {
    //smsc_log_debug(logger, "%p: %p response parsed", this, cx);
    s->Abort();
    service = NULL;
    delete s;
    action = PROCESS_RESPONSE;
    } else {
      //smsc_log_debug(logger, "%p: %p request parsed", this, cx);
      std::string tid = command.getTransactionId();
      command.setTransactionId(transaction_id);
      transaction_id = tid;
      action = PROCESS_REQUEST;
    }
    result = 0;
    return true;
  } 

  if (action == READ_RESPONSE) {
    if (is_vasp) {
      command.createVASPError(transaction_id, "4004", "Validation Error");
    } else {
      command.createRSError(transaction_id, "4004", "Validation Error");
    }
    action = PROCESS_RESPONSE;
    return true;
  }

  const MmsMsg *msg = command.getMmsMsg();
  if (msg) {
    command.createResponse("4004", "Validation Error");
  } else {
    std::string tid = command.getTransactionId();
    if (tid.empty()) {
      return false;
    } else {
      if (is_vasp) {
        command.createVASPError(tid, "4004", "Validation Error");
      } else {
        command.createRSError(tid, "4004", "Validation Error");
      }
    }
  }
  action = PROCESS_RESPONSE;
  return true;

}
*/
//const char* MmsContext::getHost() {
  //return trc.host.c_str();
//}

//unsigned int MmsContext::getPort() {
  //return trc.port;
//}

}//mms
}//transport
}//scag
