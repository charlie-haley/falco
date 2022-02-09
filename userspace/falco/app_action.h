/*
Copyright (C) 2022 The Falco Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <string>
#include <list>

namespace falco {
namespace app {

// Current set of actions:
// One-offs:
//  - parse command line options
//  - display help
//  - print version info
//  - print ignored events
//  - list plugins
//  - list all fields
//  - print support
//  - describe rule(s)

// "main" actions:
//  - setup signal handlers
//  - create/configure inspector
//  - load plugins
//  - create/configure falco engine
//  - validate rules files
//  - read config
//  - load rules files
//  - daemonize
//  - initialize outputs
//  - start grpc server
//  - start webserver
//  - read events from source (trace or live), pass to falco engine

// This class represents an "action" e.g. a chunk of code to execute
// as a part of running the falco application. Examples of actions are:
//   - initializing/configuring the inspector
//   - loading/configuring plugins
//   - reading events from a trace file or live event source
//
// Actions also include "one off" actions for things like --help
// output, --list fields, etc.
//
// There's no attempt in this version to distribute state
// (e.g. inspectors, lists of plugins, etc) across actions. The
// expectation is that all state that needs to be used across actions
// is held in the provided application object and actions know which
// state they should create and destroy.

class action {
public:

	struct run_result {
		// If true, the action completed successfully.
		bool success;

		// If success==false, details on the error.
		std::string errstr;

		// If true, subsequent actions should be performed. If
		// false, subsequent actions should *not* be performed
		// and falco should tear down/exit.
		bool proceed;
	};

	action();
	virtual ~action();

	// Return the name of the action. Only used for logging
	// purposes and to use in prerequsites().
	virtual const std::string &name() = 0;

	// Return a list of action names that *must* run before this
	// action is run.
	virtual const std::list<std::string> &prerequsites() = 0;

	// Initialize any state in the application that might be
	// shared with other components. This might include creating
	// inspectors, falco engines, etc.
	virtual void init();

	// Destroy any state created in init()
	virtual void deinit();

	// Return true if this action should run, given the state held
	// in application. For example, an action "print help and
	// exit" should return true if the command line option --help
	// was specified, false otherwise.
	virtual bool should_run();

	// Perform the action. The returned run_result holds the
	// result of the action and whether later actions should
	// procceed.
	virtual run_result run() = 0;
};

}; // namespace application
}; // namespace falco




