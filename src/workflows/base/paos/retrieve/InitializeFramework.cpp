/*!
 * \brief Class represents the retrieved PAOS InitializeFramework
 *
 * \copyright Copyright (c) 2014-2023 Governikus GmbH & Co. KG, Germany
 */

#include "paos/PaosType.h"
#include "paos/retrieve/InitializeFramework.h"

using namespace governikus;

InitializeFramework::InitializeFramework(const QByteArray& pXmlData)
	: PaosMessage(PaosType::INITIALIZE_FRAMEWORK)
	, ElementDetector(pXmlData)
{
	parse();
}


void InitializeFramework::parse()
{
	const QStringList expectedElements({
				QStringLiteral("RelatesTo"),
				QStringLiteral("MessageID")
			});

	detectStartElements(expectedElements);
}


bool InitializeFramework::handleFoundElement(const QString& pElementName, const QString& pValue, const QXmlStreamAttributes& pAttributes)
{
	if (handleWSAddressingHeaders(pElementName, pValue, pAttributes))
	{
		// handled all WS addressing PAOS header stuff
	}
	return true;
}
