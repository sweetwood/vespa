// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include "frtconfigagent.h"
#include <vespa/config/common/trace.h>

#include <vespa/log/log.h>
LOG_SETUP(".config.frt.frtconfigagent");

namespace config {

FRTConfigAgent::FRTConfigAgent(const IConfigHolder::SP & holder, const TimingValues & timingValues)
    : _holder(holder),
      _timingValues(timingValues),
      _configState(),
      _latest(),
      _waitTime(0),
      _numConfigured(0),
      _failedRequests(0),
      _nextTimeout(_timingValues.initialTimeout)
{
}

FRTConfigAgent::~FRTConfigAgent() {}

void
FRTConfigAgent::handleResponse(const ConfigRequest & request, ConfigResponse::UP response)
{
    if (LOG_WOULD_LOG(spam)) {
        const ConfigKey & key(request.getKey());
        LOG(spam, "current state for %s: generation %ld md5 %s", key.toString().c_str(), _configState.generation, _configState.md5.c_str());
    }
    if (response->validateResponse() && !response->isError()) {
        handleOKResponse(std::move(response));
    } else {
        handleErrorResponse(request, std::move(response));
    }
}

void
FRTConfigAgent::handleOKResponse(ConfigResponse::UP response)
{
    _failedRequests = 0;
    response->fill();
    if (LOG_WOULD_LOG(spam)) {
        LOG(spam, "trace(%s)", response->getTrace().toString().c_str());
    }

    ConfigState newState = response->getConfigState();
    bool isNewGeneration = newState.isNewerGenerationThan(_configState);
    if (isNewGeneration) {
        handleUpdatedGeneration(response->getKey(), newState, response->getValue());
    }
    setWaitTime(_timingValues.successDelay, 1);
    _nextTimeout = _timingValues.successTimeout;
}

void
FRTConfigAgent::handleUpdatedGeneration(const ConfigKey & key, const ConfigState & newState, const ConfigValue & configValue)
{
    if (LOG_WOULD_LOG(spam)) {
        LOG(spam, "new generation %ld for key %s", newState.generation, key.toString().c_str());
    }
    _configState.generation = newState.generation;
    bool hasDifferentPayload = newState.hasDifferentPayloadFrom(_configState);
    if (hasDifferentPayload) {
        if (LOG_WOULD_LOG(spam)) {
            LOG(spam, "new payload for key %s, existing md5(%s), new md5(%s)", key.toString().c_str(), _configState.md5.c_str(), newState.md5.c_str());
        }
        _configState.md5 = newState.md5;
        _latest = configValue;
    }

    if (LOG_WOULD_LOG(spam)) {
        LOG(spam, "updating holder for key %s, payload changed: %d", key.toString().c_str(), hasDifferentPayload ? 1 : 0);
    }
    _holder->handle(ConfigUpdate::UP(new ConfigUpdate(_latest, hasDifferentPayload, _configState.generation)));
    _numConfigured++;
}

void
FRTConfigAgent::handleErrorResponse(const ConfigRequest & request, ConfigResponse::UP response)
{
    _failedRequests++;
    int multiplier = std::min(_failedRequests, _timingValues.maxDelayMultiplier);
    setWaitTime(_numConfigured > 0 ? _timingValues.configuredErrorDelay : _timingValues.unconfiguredDelay, multiplier);
    _nextTimeout = _timingValues.errorTimeout;
    const ConfigKey & key(request.getKey());
    LOG(info, "Error response or no response from config server (key: %s) (errcode=%d, validresponse:%d), trying again in %ld milliseconds", key.toString().c_str(), response->errorCode(), response->hasValidResponse() ? 1 : 0, _waitTime);
}

void
FRTConfigAgent::setWaitTime(uint64_t delay, int multiplier)
{
    uint64_t prevWait = _waitTime;
    _waitTime = _timingValues.fixedDelay + (multiplier * delay);
    LOG(spam, "Adjusting waittime from %ld to %ld", prevWait, _waitTime);
}

uint64_t FRTConfigAgent::getTimeout() const { return _nextTimeout; }
uint64_t FRTConfigAgent::getWaitTime() const { return _waitTime; }
const ConfigState & FRTConfigAgent::getConfigState() const { return _configState; }

}
