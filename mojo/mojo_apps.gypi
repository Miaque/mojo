# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      # GN version: //mojo/apps/js
      #             //mojo/apps/js/bindings
      #             //mojo/apps/js/bindings/gl
      'target_name': 'mojo_js_lib',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../gin/gin.gyp:gin',
        '../ui/gl/gl.gyp:gl',
        '../v8/tools/gyp/v8.gyp:v8',
        'mojo_base.gyp:mojo_common_lib',
        'mojo_base.gyp:mojo_environment_chromium',
        'mojo_base.gyp:mojo_js_bindings_lib',
        'mojo_gles2_lib',
        'services/public/mojo_services_public.gyp:mojo_native_viewport_bindings',
      ],
      'includes': [
        'mojo_public_gles2_for_loadable_module.gypi',
      ],
      'export_dependent_settings': [
        '../base/base.gyp:base',
        '../gin/gin.gyp:gin',
        'mojo_base.gyp:mojo_common_lib',
        'mojo_gles2_lib',
        'services/public/mojo_services_public.gyp:mojo_native_viewport_bindings',
      ],
      'sources': [
        'apps/js/mojo_runner_delegate.cc',
        'apps/js/mojo_runner_delegate.h',
        'apps/js/bindings/threading.cc',
        'apps/js/bindings/threading.h',
        'apps/js/bindings/gl/context.cc',
        'apps/js/bindings/gl/context.h',
        'apps/js/bindings/gl/module.cc',
        'apps/js/bindings/gl/module.h',
        'apps/js/bindings/monotonic_clock.cc',
        'apps/js/bindings/monotonic_clock.h',
      ],
    },
    {
      # GN version: //mojo/apps/js/test:js_to_cpp_bindings
      'target_name': 'mojo_apps_js_bindings',
      'type': 'static_library',
      'sources': [
        'apps/js/test/js_to_cpp.mojom',
      ],
      'includes': [ 'public/tools/bindings/mojom_bindings_generator.gypi' ],
      'export_dependent_settings': [
        'public/mojo_public.gyp:mojo_cpp_bindings',
      ],
      'dependencies': [
        'public/mojo_public.gyp:mojo_cpp_bindings',
      ],
    },
    {
      # GN version: //mojo/apps/js/test/mojo_apps_js_unittests
      'target_name': 'mojo_apps_js_unittests',
      'type': 'executable',
      'dependencies': [
        '../gin/gin.gyp:gin_test',
        'edk/mojo_edk.gyp:mojo_common_test_support',
        'edk/mojo_edk.gyp:mojo_run_all_unittests',
        'mojo_apps_js_bindings',
        'mojo_base.gyp:mojo_common_lib',
        'mojo_js_lib',
        'public/mojo_public.gyp:mojo_public_test_interfaces',
      ],
      'sources': [
        'apps/js/test/handle_unittest.cc',
        'apps/js/test/js_to_cpp_unittest.cc',
        'apps/js/test/run_apps_js_tests.cc',
      ],
    },
    {
      # GN version: //mojo/apps/js/test:mojo_js_apps_lib
      'target_name': 'mojo_js_apps_lib',
      'type': 'static_library',
      'export_dependent_settings': [
        'public/mojo_public.gyp:mojo_cpp_bindings',
      ],
      'dependencies': [
        'mojo_apps_js_bindings',
        'mojo_base.gyp:mojo_application_chromium',
        'mojo_js_lib',
        'public/mojo_public.gyp:mojo_cpp_bindings',
        'public/mojo_public.gyp:mojo_utility',
      ],
      'sources': [
        'apps/js/application_delegate_impl.cc',
        'apps/js/js_app.cc',
        'apps/js/mojo_bridge_module.cc',
      ],
    },
    {
      # GN version: //mojo/apps/js:mojo_js_content_handler
      'target_name': 'mojo_js_content_handler',
      'type': 'loadable_module',
      'dependencies': [
        'services/public/mojo_services_public.gyp:mojo_content_handler_bindings',
        'mojo_js_apps_lib',
        '<(mojo_system_for_loadable_module)',
      ],
      'sources': [
        'apps/js/content_handler_impl.cc',
        'apps/js/content_handler_main.cc',
      ],
    },
    {
      # GN version: //mojo/apps/js:mojo_js_standalone
      'target_name': 'mojo_js_standalone',
      'type': 'loadable_module',
      'dependencies': [
        'mojo_js_apps_lib',
        '<(mojo_system_for_loadable_module)',
      ],
      'sources': [
        'apps/js/standalone_main.cc',
      ],
    },
  ],
  'conditions': [
    ['test_isolation_mode != "noop"', {
      'targets': [
        {
          'target_name': 'mojo_apps_js_unittests_run',
          'type': 'none',
          'dependencies': [
            'mojo_apps_js_unittests',
          ],
          'includes': [
            '../build/isolate.gypi',
          ],
          'sources': [
            'mojo_apps_js_unittests.isolate',
          ],
        },
      ],
    }],
  ],
}
