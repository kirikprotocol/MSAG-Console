#ifndef SMSC_UTIL_XML_DTDRESOLVER
#define SMSC_UTIL_XML_DTDRESOLVER

#include <sys/stat.h>

#include <sax/EntityResolver.hpp>
#include <sax/SAXException.hpp>
#include <framework/LocalFileInputSource.hpp>
#include <util/XMLString.hpp>

namespace smsc {
namespace util {
namespace xml {

class DtdResolver : public EntityResolver {
public:
	/**
	  * Allow the application to resolve external entities.
	  *
	  * <p>The Parser will call this method before opening any external
	  * entity except the top-level document entity (including the
	  * external DTD subset, external entities referenced within the
	  * DTD, and external entities referenced within the document
	  * element): the application may request that the parser resolve
	  * the entity itself, that it use an alternative URI, or that it
	  * use an entirely different input source.</p>
	  *
	  * <p>Application writers can use this method to redirect external
	  * system identifiers to secure and/or local URIs, to look up
	  * public identifiers in a catalogue, or to read an entity from a
	  * database or other input source (including, for example, a dialog
	  * box).</p>
	  *
	  * <p>If the system identifier is a URL, the SAX parser must
	  * resolve it fully before reporting it to the application.</p>
	  *
	  * @param publicId The public identifier of the external entity
	  *        being referenced, or null if none was supplied.
	  * @param systemId The system identifier of the external entity
	  *        being referenced.
	  * @return An InputSource object describing the new input source,
	  *         or null to request that the parser open a regular
	  *         URI connection to the system identifier.
	  * @exception SAXException Any SAX exception, possibly
	  *            wrapping another exception.
	  * @exception IOException An IO exception,
	  *            possibly the result of creating a new InputStream
	  *            or Reader for the InputSource.
	  * @see InputSource#InputSource
	  */
	virtual InputSource* resolveEntity(const XMLCh *const publicId, const XMLCh *const systemId)
	{
		if (XMLString::endsWith(systemId, DOMString(".dtd").rawBuffer()))
		{
			int idx = XMLString::lastIndexOf(systemId, '/');
			if (idx == -1)
				idx = XMLString::lastIndexOf(systemId, '\\');
			if (idx != -1)
			{
				int len = XMLString::stringLen(systemId);
				XMLCh tmp[len+1];
				XMLString::subString(tmp, systemId, idx+1, len);
				return createInputSource(tmp);
			} else {
				return createInputSource(systemId);
			}
		} else
			return 0;
	}

private:
	InputSource * createInputSource(const XMLCh * const dtdName)
	{
		struct stat s;
		const char prefixChars[] = "../conf/";
		const size_t prefixLen = strlen(prefixChars);
		const size_t dtdNameLen = XMLString::stringLen(dtdName);
		XMLCh tmpDtdName[prefixLen + dtdNameLen + 1];
		XMLString::copyNString(tmpDtdName, DOMString(prefixChars).rawBuffer(), prefixLen);
		XMLString::copyNString(tmpDtdName + prefixLen, dtdName, dtdNameLen);
		tmpDtdName[prefixLen + dtdNameLen] = 0;
		
		std::auto_ptr<char> dtdNameTranscodedToCallCFunctionStat(XMLString::transcode(tmpDtdName));
		if (stat(dtdNameTranscodedToCallCFunctionStat.get(), &s) == 0)
		{
			return new LocalFileInputSource(tmpDtdName);
		}
		else
		{
			return new LocalFileInputSource(dtdName);
		}
	}
};


}
}
}
#endif // ifndef SMSC_UTIL_XML_DTDRESOLVER
