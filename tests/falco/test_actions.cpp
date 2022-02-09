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

#include <catch.hpp>

// Test actions just record the order they were run (or skipped)
class test_action : public falco::app::action {
public:

	static std::list<std::string> s_actions_run;
	static std::list<std::string> s_actions_skipped;

	test_action(const std::string &name,
		    const std::list<std::string> &prerequsites,
		    bool should_run,
		    run_result res)
		: m_name(name),
		  m_should_run(should_run),
		  m_prerequsites(prerequsites),
		  m_res(res)
		{
		}

	~test_action()
		{
		}

	const std::string &name()
	{
		return m_name;
	}

	const std::list<std::string> &prerequsites()
	{
		return m_prerequsites;
	}

	bool should_run()
	{
		if(!m_should_run)
		{
			s_actions_skipped.push_back(m_name);
		}

		return m_should_run;
	}

	run_result run()
	{
		s_actions_run.push_back(m_name);
		return m_res;
	}

private:
	std::string m_name;
	bool m_should_run;
	std::list<std::string> m_prerequsites;
	run_result m_res;
};

std::list<std::string> test_action::s_actions_run;
std::list<std::string> test_action::s_actions_skipped;

static std::list<std::string> no_prerequsites;
static std::list<std::string> actions_empty;

static falco::app::action::run_result success_proceed{true, "", true};

TEST_CASE("action manager can add and run actions", "[actions]")
{
	falco::app::action_manager amgr;

	std::shared_ptr<test_action> first = std::make_shared<test_action>(std::string("first"),
									   no_prerequsites,
									   true,
									   success_proceed);

	std::shared_ptr<test_action> second = std::make_shared<test_action>(std::string("second"),
									    no_prerequsites,
									    true,
									    success_proceed);

	std::list<std::string> depend_first_prereq = {"first"};
	std::shared_ptr<test_action> depend_first = std::make_shared<test_action>(std::string("depend_first"),
										  depend_first_prereq,
										  true,
										  success_proceed);

	SECTION("Two independent actions")
	{
		test_action::s_actions_run.clear();
		test_action::s_actions_skipped.clear();

		amgr.add(first);
		amgr.add(second);

		amgr.run();

		REQUIRE(test_action::s_actions_skipped == actions_empty);

		// Can't compare to any direct list as order is not guaranteed
		REQUIRE(std::find(test_action::s_actions_run.begin(),
				  test_action::s_actions_run.end(),
				  std::string("first")) != test_action::s_actions_run.end());
		REQUIRE(std::find(test_action::s_actions_run.begin(),
				  test_action::s_actions_run.end(),
				  std::string("second")) != test_action::s_actions_run.end());
	}

	SECTION("Two dependent actions")
	{
		test_action::s_actions_run.clear();
		test_action::s_actions_skipped.clear();

		amgr.add(first);
		amgr.add(depend_first);

		amgr.run();

		REQUIRE(test_action::s_actions_skipped == actions_empty);

		std::list<std::string> exp_actions_run = {"first", "depend_first"};
		REQUIRE(test_action::s_actions_run == exp_actions_run);
	}

}



