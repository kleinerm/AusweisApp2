/*!
 * \brief Extension of WebSocketServer interface for local PSK connections
 *
 * \copyright Copyright (c) 2017-2023 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "WebSocketServer.h"

namespace governikus
{

class LocalWebSocketServer
	: public WebSocketServer
{
	Q_OBJECT

	public:
		~LocalWebSocketServer() override;

		virtual void setPsk(const QByteArray& pPsk) = 0;

};

} // namespace governikus
