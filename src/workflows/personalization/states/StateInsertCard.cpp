/*!
 * \copyright Copyright (c) 2021-2023 Governikus GmbH & Co. KG, Germany
 */

#include "StateInsertCard.h"

#include "Env.h"
#include "ReaderFilter.h"
#include "ReaderManager.h"

Q_DECLARE_LOGGING_CATEGORY(statemachine)

using namespace governikus;


StateInsertCard::StateInsertCard(const QSharedPointer<WorkflowContext>& pContext)
	: AbstractState(pContext)
	, GenericContextContainer(pContext)
{
}


void StateInsertCard::run()
{
	auto* readerManager = Env::getSingleton<ReaderManager>();

	const auto& readerInfos = readerManager->getReaderInfos(ReaderFilter({ReaderManagerPlugInType::SMART}));
	if (readerInfos.isEmpty())
	{
		qCWarning(statemachine) << "No Smart reader present";
		updateStatus(GlobalStatus::Code::Workflow_Smart_eID_Personalization_Failed);
		Q_EMIT fireAbort();
		return;
	}
	if (readerInfos.size() > 1)
	{
		qCWarning(statemachine) << "Multiple Smart readers present";
		updateStatus(GlobalStatus::Code::Workflow_Smart_eID_Personalization_Failed);
		Q_EMIT fireAbort();
		return;
	}

	getContext()->setReaderPlugInTypes({ReaderManagerPlugInType::SMART});
	mConnections += connect(readerManager, &ReaderManager::fireCardInfoChanged, this, &StateInsertCard::onCardInfoChanged);
	mConnections += connect(readerManager, &ReaderManager::fireStatusChanged, this, &StateInsertCard::onStatusChanged);
	readerManager->startScan(ReaderManagerPlugInType::SMART);
}


void StateInsertCard::onCardInfoChanged(const ReaderInfo& pInfo)
{
	if (pInfo.getPlugInType() != ReaderManagerPlugInType::SMART)
	{
		return;
	}

	switch (const auto type = pInfo.getCardInfo().getMobileEidType())
	{
		case MobileEidType::SE_CERTIFIED:
		case MobileEidType::SE_ENDORSED:
			break;

		case MobileEidType::UNKNOWN:
			qCWarning(statemachine) << "Smart card has an unexpected eID-Type:" << type;
			updateStatus(GlobalStatus::Code::Workflow_Smart_eID_Personalization_Failed);
			Q_EMIT fireAbort();
			return;

		case MobileEidType::HW_KEYSTORE:
			qCDebug(statemachine) << "Skipping PIN change because of eID-Type:" << type;
			Q_EMIT fireAbort();
			return;
	}

	if (!pInfo.wasShelved())
	{
		qCWarning(statemachine) << "Smart reader is present but not insertable";
		updateStatus(GlobalStatus::Code::Workflow_Smart_eID_Personalization_Failed);
		Q_EMIT fireAbort();
		return;
	}

	Env::getSingleton<ReaderManager>()->insert(pInfo, true);
	Q_EMIT fireContinue();
}


void StateInsertCard::onStatusChanged(const ReaderManagerPlugInInfo& pInfo)
{
	if (pInfo.getPlugInType() != ReaderManagerPlugInType::SMART)
	{
		return;
	}

	qCWarning(statemachine) << "Scan was started but no smart card was found";
	updateStatus(GlobalStatus::Code::Workflow_Smart_eID_Personalization_Failed);
	Q_EMIT fireAbort();
}
