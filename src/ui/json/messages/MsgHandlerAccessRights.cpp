/*!
 * \copyright Copyright (c) 2016-2021 Governikus GmbH & Co. KG, Germany
 */

#include "MsgHandlerAccessRights.h"

#include <QJsonArray>

using namespace governikus;

MsgHandlerAccessRights::MsgHandlerAccessRights(const MsgContext& pContext)
	: MsgHandler(MsgType::ACCESS_RIGHTS)
{
	Q_ASSERT(pContext.getContext<AuthContext>());
	fillAccessRights(pContext.getContext<AuthContext>());
}


MsgHandlerAccessRights::MsgHandlerAccessRights(const QJsonObject& pObj, MsgContext& pContext)
	: MsgHandler(MsgType::ACCESS_RIGHTS)
{
	auto ctx = pContext.getContext<AuthContext>();
	Q_ASSERT(ctx);

	const auto& jsonRaw = pObj[QLatin1String("chat")];
	if (jsonRaw.isUndefined())
	{
		setError(QLatin1String("'chat' cannot be undefined"));
	}
	else if (!jsonRaw.isArray())
	{
		setError(QLatin1String("Invalid 'chat' data"));
	}
	else
	{
		handleSetChatData(jsonRaw.toArray(), ctx);
	}

	fillAccessRights(ctx);
}


void MsgHandlerAccessRights::handleSetChatData(const QJsonArray& pChat, const QSharedPointer<AuthContext>& pContext)
{
	Q_ASSERT(pContext);

	QSet<AccessRight> effectiveChat;

	if (!pContext->getAccessRightManager()->getOptionalAccessRights().isEmpty())
	{
		for (const auto& entry : pChat)
		{
			if (entry.isString())
			{
				const auto& func = [this, &pContext, &effectiveChat](AccessRight pRight){
							if (pContext->getAccessRightManager()->getOptionalAccessRights().contains(pRight))
							{
								effectiveChat += pRight;
							}
							else
							{
								setError(QLatin1String("Entry in 'chat' data is not available"));
							}
						};

				if (!AccessRoleAndRightsUtil::fromTechnicalName(entry.toString(), func))
				{
					setError(QLatin1String("Entry in 'chat' data is invalid"));
				}
			}
			else
			{
				setError(QLatin1String("Entry in 'chat' data needs to be string"));
			}
		}
	}
	else
	{
		setError(QLatin1String("No optional access rights available"));
	}

	if (!mJsonObject.contains(QLatin1String("error")))
	{
		*pContext->getAccessRightManager() = effectiveChat;
	}
}


QJsonArray MsgHandlerAccessRights::getAccessRights(const QSet<AccessRight>& pRights) const
{
	QJsonArray array;

	QList<AccessRight> accessRights = pRights.values();
	std::sort(accessRights.rbegin(), accessRights.rend());
	for (auto entry : qAsConst(accessRights))
	{
		const QLatin1String name = AccessRoleAndRightsUtil::toTechnicalName(entry);
		if (name.size())
		{
			array += name;
		}
	}

	return array;
}


void MsgHandlerAccessRights::fillAccessRights(const QSharedPointer<const AuthContext>& pContext)
{
	Q_ASSERT(pContext);

	QJsonObject chat;
	const auto& accessRightManager = pContext->getAccessRightManager();
	chat[QLatin1String("required")] = getAccessRights(accessRightManager->getRequiredAccessRights());
	chat[QLatin1String("optional")] = getAccessRights(accessRightManager->getOptionalAccessRights());
	chat[QLatin1String("effective")] = getAccessRights(accessRightManager->getEffectiveAccessRights());

	mJsonObject[QLatin1String("chat")] = chat;
	const auto& transactionInfo = pContext->getDidAuthenticateEac1()->getTransactionInfo();
	if (!transactionInfo.isEmpty())
	{
		mJsonObject[QLatin1String("transactionInfo")] = transactionInfo;
	}

	const QJsonObject& aux = getAuxiliaryData(pContext);
	if (!aux.isEmpty())
	{
		mJsonObject[QLatin1String("aux")] = aux;
	}
}


QJsonObject MsgHandlerAccessRights::getAuxiliaryData(const QSharedPointer<const AuthContext>& pContext) const
{
	QJsonObject obj;

	const auto& eac1 = pContext->getDidAuthenticateEac1();
	if (eac1)
	{
		const auto& aux = eac1->getAuthenticatedAuxiliaryData();
		if (aux)
		{
			if (aux->hasAgeVerificationDate())
			{
				obj[QLatin1String("ageVerificationDate")] = aux->getAgeVerificationDate().toString(Qt::ISODate);
				obj[QLatin1String("requiredAge")] = aux->getRequiredAge();
			}

			if (aux->hasValidityDate())
			{
				obj[QLatin1String("validityDate")] = aux->getValidityDate().toString(Qt::ISODate);
			}

			if (aux->hasCommunityID())
			{
				obj[QLatin1String("communityId")] = QString::fromUtf8(aux->getCommunityID());
			}
		}
	}

	return obj;
}


void MsgHandlerAccessRights::setError(const QLatin1String pError)
{
	mJsonObject[QLatin1String("error")] = pError;
}
