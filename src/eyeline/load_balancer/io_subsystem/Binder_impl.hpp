template <class BIND_REQUEST>
void
Binder::bind(const LinkId& link_set_id_to_smsc, const BIND_REQUEST& bind_request)
{
  SetOfNotBindedConnections* notBindedConns=NULL;
  smsc_log_debug(_logger, "Binder::bind::: try bind links in linkset with id='%s', BindRequest=[%s]",
                 link_set_id_to_smsc.toString().c_str(), bind_request.toString().c_str());
  {
    smsc::core::synchronization::MutexGuard synchronize(_knownSetsOfNotBindedConnsLock);
    linksetid_to_notbinded_conns_map_t::iterator iter = _knownSetsOfNotBindedConns.find(link_set_id_to_smsc);
    if ( iter == _knownSetsOfNotBindedConns.end() ) {
      smsc_log_error(_logger, "Binder::bind::: there isn't established connection with id=[%s]", link_set_id_to_smsc.toString().c_str());
      return;
    }

    notBindedConns = iter->second;
    if ( notBindedConns->isEmpty() ) {
      delete notBindedConns;
      _knownSetsOfNotBindedConns.erase(iter);
      return;
    }
  }

  notBindedConns->bindConnections(bind_request);
}

