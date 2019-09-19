/*
 Flexisip, a flexible SIP proxy server with media capabilities.
 Copyright (C) 2010 Belledonne Communications SARL, All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <array>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include "flexisip/module.hh"

#include "auth/flexisip-auth-module-base.hh"

namespace flexisip {

/**
 * Base class for Flexisip authentication modules.
 */
class ModuleAuthenticationBase : public Module {
public:
	ModuleAuthenticationBase(Agent *agent);
	~ModuleAuthenticationBase() = default;

protected:
	/**
	 * This exception is globally caught by ModuleAuthenticationBase::onRequest()
	 * causing onRequest() return. It is to used in any sub-functions
	 * of onRequest() in order to stop the request event processing
	 * and pass to the next Flexisip module.
	 */
	class StopRequestProcessing : public std::exception {};

	void onDeclare(GenericStruct *root) override;
	void onLoad(const GenericStruct *root) override;
	void onRequest(std::shared_ptr<RequestSipEvent> &ev) override;
	void onResponse(std::shared_ptr<ResponseSipEvent> &ev) override {}

	/**
	 * Override this method to specify the specialization of #FlexisipAuthModuleBase to instantiate.
	 */
	virtual FlexisipAuthModuleBase *createAuthModule(const std::string &domain, int nonceExpire, bool qopAuth) = 0;
	/**
	 * @brief Create and configure a #FlexisipAuthStatus according the information extracted from ev.
	 *
	 * This method may be overridden in order to instantiate a specialization of #FlexisipAuthStatus. Should it be,
	 * the overriding method might call #configureAuthStatus() for configuring the base of the returned object.
	 */
	virtual FlexisipAuthStatus *createAuthStatus(const std::shared_ptr<RequestSipEvent> &ev);
	/**
	 * Called by createAuthStatus() for setting #FlexisipAuthStatus attribute for the event request information.
	 */
	void configureAuthStatus(FlexisipAuthStatus &as, const std::shared_ptr<RequestSipEvent> &ev);


	/**
	 * These two methods might be overridden to customize onRequest().
	 */
	virtual void validateRequest(const std::shared_ptr<RequestSipEvent> &request);
	virtual void processAuthentication(const std::shared_ptr<RequestSipEvent> &request, FlexisipAuthModuleBase &am);

	/**
	 * Called by onRequest() for getting a #FlexisipAuthModuleBase instance from a domain name.
	 */
	FlexisipAuthModuleBase *findAuthModule(const std::string name);

	/**
	 * This method is called synchronously or asynchronously on result of AuthModule::verify() method.
	 * It calls onSuccess() and errorReply() according the authentication result.
	 */
	void processAuthModuleResponse(AuthStatus &as);
	virtual void onSuccess(const FlexisipAuthStatus &as);
	virtual void errorReply(const FlexisipAuthStatus &as);

	/**
	 * Test whether a string match a valid algorithm in specified by sValidAlgos.
	 */
	static bool validAlgo(const std::string &algo);

protected:
	std::map<std::string, std::unique_ptr<FlexisipAuthModuleBase>> mAuthModules;
	std::list<std::string> mAlgorithms;
	auth_challenger_t mRegistrarChallenger;
	auth_challenger_t mProxyChallenger;
	std::string mRealmRegexStr;
	std::regex mRealmRegex;
	std::shared_ptr<SipBooleanExpression> mNo403Expr;

	static const std::array<std::string, 2> sValidAlgos;
};

}
