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

#include "app_action_manager.h"
#include "logger.h"
#include "falco_common.h"

#include <algorithm>
#include <list>
#include <memory>
#include <string>

namespace falco {
namespace app {

action_manager::action_manager()
{
}

action_manager::~action_manager()
{
}

void action_manager::add(std::shared_ptr<action> act)
{
	m_actions.push_back(act);
}

static bool compare_actions(const std::shared_ptr<action> &a, const std::shared_ptr<action> &b)
{
	bool a_prereq_b = (std::find(b->prerequsites().begin(),
				     b->prerequsites().end(),
				     a->name()) != b->prerequsites().end());

	bool b_prereq_a = (std::find(a->prerequsites().begin(),
				     a->prerequsites().end(),
				     b->name()) != a->prerequsites().end());

	// If both are prerequsites of each other, there is a cycle
	// and throw an exception.
	if(a_prereq_b && b_prereq_a)
	{
		throw falco_exception(std::string("Dependency cycle for actions ") + a->name() + " and " + b->name());
	}

	// If neither are, just sort on the name
	if(!a_prereq_b && !b_prereq_a)
	{
		return (a->name() < b->name());
	}

	// If b is a prereq of a, a is "less"
	if(b_prereq_a)
	{
		return true;
	}

	// a must be a prereq of b. it is not less
	return false;
}


void action_manager::run()
{
	// Order the actions according to precedence
	std::make_heap(m_actions.begin(), m_actions.end(), compare_actions);

	for(auto &act : m_actions)
	{
		falco_logger::log(LOG_DEBUG, string("Considering action ") + act->name());

		act->init();

		if(act->should_run())
		{
			falco_logger::log(LOG_DEBUG, string("Running action ") + act->name());

			action::run_result res = act->run();

			if(!res.success)
			{
				fprintf(stderr, "Could not complete %s: %s\n", act->name().c_str(), res.errstr.c_str());
			}

			if(!res.proceed)
			{
				break;
			}
		}
		else
		{
			falco_logger::log(LOG_DEBUG, string("Skipping action ") + act->name());
		}
	}

	return;
}

}; // namespace application
}; // namespace falco
