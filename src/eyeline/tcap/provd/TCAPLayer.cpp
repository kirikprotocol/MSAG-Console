#include <memory>

#include "TCAPLayer.hpp"
#include "TDialogueServiceDataRegistry.hpp"
#include "AppCtxSMRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TCAPLayer::TCAPLayer()
  : _logger(smsc::logger::Logger::getInstance("tcap.provd")),
    _outPrimitivesProcessor(NULL), _msgReaders(NULL), _wasInitialized(false),
    _maxDialoguesCount(0), _dialogueTimeout(0), _rosTimeout(0), _suaApi(NULL)
{}

TCAPLayer::~TCAPLayer()
{
  shutdown();
  delete _outPrimitivesProcessor;
  delete _msgReaders;
  delete _suaApi;
}

void
TCAPLayer::initialize(smsc::util::config::ConfigView& tc_provider_cfg_section)
{
  if ( !_wasInitialized ) {
    _maxDialoguesCount = tc_provider_cfg_section.getInt("maxDialoguesCount");
    _dialogueTimeout = tc_provider_cfg_section.getInt("dialogueTimeout");
    _rosTimeout = tc_provider_cfg_section.getInt("rosTimeout");

    std::auto_ptr<smsc::util::config::ConfigView> sua_cfg_section(tc_provider_cfg_section.getSubConfig("sua"));

    _suaApi = new sua::libsua::SuaUser();
    _suaApi->sua_init(sua_cfg_section.get());

    unsigned msgReadersCount = tc_provider_cfg_section.getInt("msgReadersCount");
    _msgReaders = new MsgReadersPool(msgReadersCount, _suaApi);
    _msgReaders->startup();

    _outPrimitivesProcessor = new OutPrimitivesProcessor(_suaApi);
    _wasInitialized = true;
  }
}

void
TCAPLayer::shutdown()
{
  _msgReaders->shutdown();
}

TCAPIface*
TCAPLayer::getTCAPIface()
{
  return this;
}

void
TCAPLayer::registerAppCTX(const EncodedOID& ctx,
                          TDlgHandlerIfaceFactory* indHndlrsFactory,
                          unsigned int dialogueTimeout)
{
  AppCtxSMRegistry::getInstance().registerDialogueHandlerFactory(ctx, indHndlrsFactory, dialogueTimeout);
}

TDialogueId
TCAPLayer::openDialogue(TDlgHandlerIface* tIndicationHandlers, unsigned int dialogueTimeout)
{
  return TDialogueServiceDataRegistry::getInstance().createTDialogueServiceData(tIndicationHandlers, dialogueTimeout)->getDialogueId();
}

void
TCAPLayer::updateDialogue(const TC_Begin_Req & req_primitive)
{
  _outPrimitivesProcessor->updateDialogue(req_primitive);
}

void
TCAPLayer::updateDialogue(const TC_Cont_Req & req_primitive)
{
  _outPrimitivesProcessor->updateDialogue(req_primitive);
}

void
TCAPLayer::updateDialogue(const TC_End_Req & req_primitive)
{
  _outPrimitivesProcessor->updateDialogue(req_primitive);
}

void
TCAPLayer::updateDialogue(const TC_UAbort_Req & req_primitive)
{
  _outPrimitivesProcessor->updateDialogue(req_primitive);
}

void
TCAPLayer::updateDialogue(const TC_PAbort_Req & req_primitive)
{
  _outPrimitivesProcessor->updateDialogue(req_primitive);
}

void
TCAPLayer::sendPrimitive(const TPAbortReqComposer & req_composer, unsigned int link_num,
                         const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const
{
  _outPrimitivesProcessor->sendPrimitive(req_composer, link_num, src_addr, dst_addr);
}

void
TCAPLayer::sendPrimitive(const TUAbortReqComposer & req_composer, unsigned int link_num,
                         const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const
{
  _outPrimitivesProcessor->sendPrimitive(req_composer, link_num, src_addr, dst_addr);
}

void
TCAPLayer::closeDialogue(const TDialogueId& tDialogueId)
{
}

}}}
