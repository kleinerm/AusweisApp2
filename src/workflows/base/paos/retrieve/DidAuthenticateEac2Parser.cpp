/*!
 * \copyright Copyright (c) 2014-2021 Governikus GmbH & Co. KG, Germany
 */

#include "paos/retrieve/DidAuthenticateEac2Parser.h"

#include "paos/element/ConnectionHandleParser.h"
#include "paos/invoke/PaosCreator.h"
#include "paos/retrieve/DidAuthenticateEac2.h"

#include <QDebug>


using namespace governikus;


DidAuthenticateEac2Parser::DidAuthenticateEac2Parser()
	: PaosParser(QStringLiteral("DIDAuthenticate"))
{
}


DidAuthenticateEac2Parser::~DidAuthenticateEac2Parser()
{
}


PaosMessage* DidAuthenticateEac2Parser::parseMessage()
{
	mDidAuthenticateEac2.reset(new DIDAuthenticateEAC2());

	bool isConnectionHandleNotSet = true;
	QString didName;

	while (readNextStartElement())
	{
		qCDebug(paos) << mXmlReader->name();
		if (mXmlReader->name() == QLatin1String("ConnectionHandle"))
		{
			if (assertNoDuplicateElement(isConnectionHandleNotSet))
			{
				isConnectionHandleNotSet = false;
				ConnectionHandleParser parser(mXmlReader);
				mDidAuthenticateEac2->setConnectionHandle(parser.parse());
				mParseError |= parser.parserFailed();
			}
		}
		else if (mXmlReader->name() == QLatin1String("DIDName"))
		{
			if (readUniqueElementText(didName))
			{
				mDidAuthenticateEac2->setDidName(didName);
			}
		}
		else if (mXmlReader->name() == QLatin1String("AuthenticationProtocolData"))
		{
			QString ns = PaosCreator::getNamespace(PaosCreator::Namespace::XSI);
			const auto value = mXmlReader->attributes().value(ns, QStringLiteral("type"));
			if (value.endsWith(QLatin1String("EAC2InputType")))
			{
				mDidAuthenticateEac2->setEac2InputType(parseEac2InputType());
			}
		}
		else
		{
			qCWarning(paos) << "Unknown element:" << mXmlReader->name();
			mXmlReader->skipCurrentElement();
		}
	}

	return mParseError ? nullptr : mDidAuthenticateEac2.take();
}


Eac2InputType DidAuthenticateEac2Parser::parseEac2InputType()
{
	Eac2InputType eac2;

	QString ephemeralPublicKey, signature;
	while (readNextStartElement())
	{
		qCDebug(paos) << mXmlReader->name();
		if (mXmlReader->name() == QLatin1String("Certificate"))
		{
			parseCertificate(eac2);
		}
		else if (mXmlReader->name() == QLatin1String("EphemeralPublicKey"))
		{
			parseEphemeralPublicKey(eac2, ephemeralPublicKey);
		}
		else if (mXmlReader->name() == QLatin1String("Signature"))
		{
			parseSignature(eac2, signature);
		}
		else
		{
			qCWarning(paos) << "Unknown element:" << mXmlReader->name();
			mXmlReader->skipCurrentElement();
		}
	}

	assertMandatoryElement(eac2.getEphemeralPublicKey(), "EphemeralPublicKey");

	return eac2;
}


void DidAuthenticateEac2Parser::parseCertificate(Eac2InputType& pEac2)
{
	const QByteArray hexCvc = readElementText().toLatin1();
	if (auto cvc = CVCertificate::fromHex(hexCvc))
	{
		pEac2.appendCvcert(cvc);
		pEac2.appendCvcertAsBinary(QByteArray::fromHex(hexCvc));
	}
	else
	{
		qCCritical(paos) << "Cannot parse Certificate";
		mParseError = true;
	}
}


void DidAuthenticateEac2Parser::parseEphemeralPublicKey(Eac2InputType& pEac2, QString& pEphemeralPublicKey)
{
	if (readUniqueElementText(pEphemeralPublicKey))
	{
		pEac2.setEphemeralPublicKey(pEphemeralPublicKey);
	}
}


void DidAuthenticateEac2Parser::parseSignature(Eac2InputType& pEac2, QString& pSignature)
{
	if (readUniqueElementText(pSignature))
	{
		pEac2.setSignature(pSignature);
	}
}
