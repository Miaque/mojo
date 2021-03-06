// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import "dart:sky.internals" as internals;

import "package:mojo/application.dart";
import "package:mojom/mojo/service_provider.mojom.dart";
import 'package:mojo/core.dart' as core;

import "embedder.dart";

ApplicationConnection _initConnection() {
  int rawHandle = internals.takeServicesProvidedByEmbedder();
  core.MojoHandle proxyHandle = new core.MojoHandle(rawHandle);
  ServiceProviderProxy serviceProvider = null;
  if (proxyHandle.isValid) serviceProvider =
      new ServiceProviderProxy.fromHandle(proxyHandle);
  return new ApplicationConnection(null, serviceProvider);
}

final ApplicationConnection _connection = _initConnection();

void requestService(String url, Object proxy) {
  if (embedder.shell == null) _connection.requestService(proxy);
  else embedder.connectToService(url, proxy);
}
