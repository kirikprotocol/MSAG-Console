#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/dom/DOMString.hpp>
#include "DOMTreeErrorReporter.h"
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

namespace smsc   {
namespace util   {
namespace config {

void DOMTreeErrorReporter::warning(const SAXParseException& toCatch)
{
	const char *filename = DOMString(toCatch.getSystemId()).transcode();
	const char *message = DOMString(toCatch.getMessage()).transcode();
	logger.warn("Warning at file \"%s\", line %d, column %d\n  Message: %s",
				filename,
				toCatch.getLineNumber(),
				toCatch.getColumnNumber(),
				message);
	delete[] filename;
	delete[] message;
}

void DOMTreeErrorReporter::error(const SAXParseException& toCatch)
{
    fSawErrors = true;
	char *filename = DOMString(toCatch.getSystemId()).transcode();
	char *message = DOMString(toCatch.getMessage()).transcode();
	logger.error("Error at file \"%s\", line %d, column %d\n  Message: %s",
				filename,
				toCatch.getLineNumber(),
				toCatch.getColumnNumber(),
				message);
	delete[] filename;
	delete[] message;
}

void DOMTreeErrorReporter::fatalError(const SAXParseException& toCatch)
{
    fSawErrors = true;
	char *filename = DOMString(toCatch.getSystemId()).transcode();
	char *message = DOMString(toCatch.getMessage()).transcode();
	logger.fatal("Fatal error at file \"%s\", line %d, column %d\n  Message: %s",
				filename,
				toCatch.getLineNumber(),
				toCatch.getColumnNumber(),
				message);
	delete[] filename;
	delete[] message;
}

void DOMTreeErrorReporter::resetErrors()
{
}

}
}
}

