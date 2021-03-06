/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sky/engine/core/loader/FrameFetchContext.h"

#include "sky/engine/core/dom/Document.h"
#include "sky/engine/core/frame/FrameConsole.h"
#include "sky/engine/core/frame/LocalFrame.h"
#include "sky/engine/core/loader/FrameLoaderClient.h"
#include "sky/engine/core/page/Page.h"

namespace blink {

FrameFetchContext::FrameFetchContext(LocalFrame* frame)
    : m_frame(frame)
{
}

void FrameFetchContext::reportLocalLoadFailed(const KURL& url)
{
    FrameLoader::reportLocalLoadFailed(m_frame, url.elidedString());
}

void FrameFetchContext::addAdditionalRequestHeaders(Document* document, ResourceRequest& request, FetchResourceType type)
{
    // FIXME(sky): remove, this is all done by the network service.
}

CachePolicy FrameFetchContext::cachePolicy(Document* document) const
{
    return CachePolicyVerify;
}

void FrameFetchContext::dispatchDidChangeResourcePriority(unsigned long identifier, ResourceLoadPriority loadPriority, int intraPriorityValue)
{
    m_frame->loaderClient()->dispatchDidChangeResourcePriority(identifier, loadPriority, intraPriorityValue);
}

void FrameFetchContext::dispatchWillSendRequest(Document* document, unsigned long identifier, ResourceRequest& request, const ResourceResponse& redirectResponse, const FetchInitiatorInfo& initiatorInfo)
{
    m_frame->loaderClient()->dispatchWillSendRequest(document, identifier, request, redirectResponse);
}

void FrameFetchContext::dispatchDidLoadResourceFromMemoryCache(const ResourceRequest& request, const ResourceResponse& response)
{
    m_frame->loaderClient()->dispatchDidLoadResourceFromMemoryCache(request, response);
}

void FrameFetchContext::dispatchDidReceiveResponse(Document* document, unsigned long identifier, const ResourceResponse& r, ResourceLoader* resourceLoader)
{
    m_frame->loaderClient()->dispatchDidReceiveResponse(document, identifier, r);
    m_frame->console().reportResourceResponseReceived(document, identifier, r);
}

void FrameFetchContext::dispatchDidReceiveData(Document*, unsigned long identifier, const char* data, int dataLength, int encodedDataLength)
{
}

void FrameFetchContext::dispatchDidDownloadData(Document*, unsigned long identifier, int dataLength, int encodedDataLength)
{
}

void FrameFetchContext::dispatchDidFinishLoading(Document* document, unsigned long identifier, double finishTime, int64_t encodedDataLength)
{
    m_frame->loaderClient()->dispatchDidFinishLoading(document, identifier);
}

void FrameFetchContext::dispatchDidFail(Document* document, unsigned long identifier, const ResourceError& error)
{
}

void FrameFetchContext::sendRemainingDelegateMessages(Document* document, unsigned long identifier, const ResourceResponse& response, int dataLength)
{
    if (!response.isNull())
        dispatchDidReceiveResponse(document, identifier, response);

    if (dataLength > 0)
        dispatchDidReceiveData(document, identifier, 0, dataLength, 0);

    dispatchDidFinishLoading(document, identifier, 0, 0);
}

} // namespace blink
