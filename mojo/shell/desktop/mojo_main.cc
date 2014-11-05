// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iostream>

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "mojo/shell/child_process.h"
#include "mojo/shell/context.h"
#include "mojo/shell/init.h"
#include "mojo/shell/mojo_url_resolver.h"
#include "mojo/shell/switches.h"

namespace {

#if defined(OS_LINUX)
// Copied from ui/gfx/switches.cc to avoid a dependency on //ui/gfx
const char kEnableHarfBuzzRenderText[] = "enable-harfbuzz-rendertext";
#endif

bool IsEmpty(const std::string& s) {
  return s.empty();
}

// The value of app_url_and_args is "<mojo_app_url> [<args>...]", where args
// is a list of "configuration" arguments separated by spaces. If one or more
// arguments are specified they will be available when the Mojo application
// is initialized. See ApplicationImpl::args().
GURL GetAppURLAndSetArgs(const std::string& app_url_and_args,
                         mojo::shell::Context* context) {
  // SplitString() returns empty strings for extra delimeter characters (' ').
  std::vector<std::string> argv;
  base::SplitString(app_url_and_args, ' ', &argv);
  argv.erase(std::remove_if(argv.begin(), argv.end(), IsEmpty), argv.end());

  if (argv.empty())
    return GURL::EmptyGURL();
  GURL app_url(argv[0]);
  if (argv.size() > 1)
    context->application_manager()->SetArgsForURL(argv, app_url);
  return app_url;
}

void RunApps(mojo::shell::Context* context) {
  const auto& command_line = *base::CommandLine::ForCurrentProcess();
  for (const auto& arg : command_line.GetArgs()) {
    std::string arg2;
#if defined(OS_WIN)
    arg2 = base::UTF16ToUTF8(arg);
#else
    arg2 = arg;
#endif
    context->Run(GetAppURLAndSetArgs(arg2, context));
  }
}

void Usage() {
  std::cerr << "Launch Mojo applications.\n";
  std::cerr
      << "Usage: mojo_shell"
      << " [--" << switches::kArgsFor << "=<mojo-app>]"
      << " [--" << switches::kContentHandlers << "=<handlers>]"
      << " [--" << switches::kEnableExternalApplications << "]"
      << " [--" << switches::kDisableCache << "]"
      << " [--" << switches::kEnableMultiprocess << "]"
      << " [--" << switches::kOrigin << "=<url-lib-path>]"
      << " [--" << switches::kURLMappings << "=from1=to1,from2=to2]"
      << " <mojo-app> ...\n\n"
      << "A <mojo-app> is a Mojo URL or a Mojo URL and arguments within "
      << "quotes.\n"
      << "Example: mojo_shell \"mojo://js_standalone test.js\".\n"
      << "<url-lib-path> is searched for shared libraries named by mojo URLs.\n"
      << "The value of <handlers> is a comma separated list like:\n"
      << "text/html,mojo://html_viewer,"
      << "application/javascript,mojo://js_content_handler\n";
}

bool ConfigureURLMappings(const std::string& mappings,
                          mojo::shell::MojoURLResolver* resolver) {
  base::StringPairs pairs;
  if (!base::SplitStringIntoKeyValuePairs(mappings, '=', ',', &pairs))
    return false;
  using StringPair = std::pair<std::string, std::string>;
  for (const StringPair& pair : pairs) {
    const GURL from(pair.first);
    const GURL to(pair.second);
    if (!from.is_valid() || !to.is_valid())
      return false;
    resolver->AddCustomMapping(from, to);
  }
  return true;
}

bool IsArgsFor(const std::string& arg, std::string* value) {
  const std::string kArgsForSwitches[] = {
    "-" + std::string(switches::kArgsFor),
    "--" + std::string(switches::kArgsFor),
  };
  for (size_t i = 0; i < arraysize(kArgsForSwitches); i++) {
    std::string argsfor_switch(kArgsForSwitches[i]);
    if (arg.compare(0, argsfor_switch.size(), argsfor_switch) == 0) {
      *value = arg.substr(argsfor_switch.size() + 1, std::string::npos);
      return true;
    }
  }
  return false;
}

}  // namespace

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);

  const base::CommandLine& command_line =
    *base::CommandLine::ForCurrentProcess();
  if (!command_line.HasSwitch(switches::kEnableExternalApplications) &&
      (command_line.HasSwitch(switches::kHelp) ||
       command_line.GetArgs().empty())) {
    Usage();
    return 0;
  }

#if defined(OS_LINUX)
  // We use gfx::RenderText from multiple threads concurrently and the pango
  // backend (currently the default on linux) is not close to threadsafe. Force
  // use of the harfbuzz backend for now.
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      kEnableHarfBuzzRenderText);
#endif
  mojo::shell::InitializeLogging();

  // TODO(vtl): Unify parent and child process cases to the extent possible.
  if (scoped_ptr<mojo::shell::ChildProcess> child_process =
          mojo::shell::ChildProcess::Create(
              *base::CommandLine::ForCurrentProcess())) {
    child_process->Main();
  } else {
    // We want the shell::Context to outlive the MessageLoop so that pipes are
    // all gracefully closed / error-out before we try to shut the Context down.
    mojo::shell::Context shell_context;
    {
      base::MessageLoop message_loop;
      shell_context.Init();

      if (command_line.HasSwitch(switches::kOrigin)) {
        shell_context.mojo_url_resolver()->SetBaseURL(
            GURL(command_line.GetSwitchValueASCII(switches::kOrigin)));
      }

      if (command_line.HasSwitch(switches::kURLMappings) &&
          !ConfigureURLMappings(
              command_line.GetSwitchValueASCII(switches::kURLMappings),
              shell_context.mojo_url_resolver())) {
        Usage();
        return 0;
      }

      // The mojo_shell --args-for command-line switch is handled specially
      // because it can appear more than once. The base::CommandLine class
      // collapses multiple occurrences of the same switch.
      for (int i = 1; i < argc; i++) {
        std::string args_for_value;
        if (IsArgsFor(argv[i], &args_for_value))
          GetAppURLAndSetArgs(args_for_value, &shell_context);
      }

      message_loop.PostTask(FROM_HERE, base::Bind(RunApps, &shell_context));
      message_loop.Run();
    }
  }
  return 0;
}
