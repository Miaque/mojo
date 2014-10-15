// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SPDY_SPDY_BUFFER_PRODUCER_H_
#define NET_SPDY_SPDY_BUFFER_PRODUCER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "net/base/net_export.h"

namespace net {

class SpdyBuffer;

// An object which provides a SpdyBuffer for writing. We pass these
// around instead of SpdyBuffers since some buffers have to be
// generated "just in time".
class NET_EXPORT_PRIVATE SpdyBufferProducer {
 public:
  SpdyBufferProducer();

  // Produces the buffer to be written. Will be called at most once.
  virtual scoped_ptr<SpdyBuffer> ProduceBuffer() = 0;

  virtual ~SpdyBufferProducer();

 private:
  DISALLOW_COPY_AND_ASSIGN(SpdyBufferProducer);
};

// A simple wrapper around a single SpdyBuffer.
class NET_EXPORT_PRIVATE SimpleBufferProducer : public SpdyBufferProducer {
 public:
  explicit SimpleBufferProducer(scoped_ptr<SpdyBuffer> buffer);

  virtual ~SimpleBufferProducer();

  virtual scoped_ptr<SpdyBuffer> ProduceBuffer() override;

 private:
  scoped_ptr<SpdyBuffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(SimpleBufferProducer);
};

}  // namespace net

#endif  // NET_SPDY_SPDY_BUFFER_PRODUCER_H_
