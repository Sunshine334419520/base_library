/**
* @Author: YangGuang
* @Date:   2018-10-15
* @Email:  guang334419520@126.com
* @Filename: at_exit.h
* @Last modified by:  YangGuang
*/
#include "base/at_exit.h"

#include "base/logging.h"

namespace base {

static AtExitManager* g_top_manager = nullptr;

static bool g_disable_manager = false;

AtExitManager::AtExitManager()
	: processing_callbacks_(false), next_manager_(g_top_manager){
	DCHECK(!g_top_manager);
	g_top_manager = this;
}

AtExitManager::~AtExitManager() {
	if (!g_top_manager) {
		LOG(logging::LogType::ERROR) << "Tried to RegisterCallback without an AtExitManager";
		return;
	}

	DCHECK_EQ(g_top_manager, this);

	if (!g_disable_manager)
		ProcessCallbacksNow();
	next_manager_ = g_top_manager;
}

void AtExitManager::RegisterCallback(AtExitCallbackType func,
									 void * param) {
	DCHECK(func);
	RegisterTask(base::BindClosure(func, param));

}

void AtExitManager::RegisterTask(base::Closure task) {
	if (!g_top_manager) {
		LOG(logging::LogType::ERROR) << "Tried to RegisterCallback without an AtExitManager";
		return;
	}

	std::lock_guard<std::mutex> lock(g_top_manager->mutex_);
	DCHECK(!g_top_manager->processing_callbacks_);
	g_top_manager->stack_.push(std::move(task));
}

void AtExitManager::ProcessCallbacksNow() {
	if (!g_top_manager) {
		LOG(logging::LogType::ERROR) << "Tried to RegisterCallback without an AtExitManager";
		return;
	}

	std::stack<base::Closure> tasks;
	{
		std::lock_guard<std::mutex> lock(g_top_manager->mutex_);
		//g_top_manager->stack_.swap(tasks);
		tasks.swap(g_top_manager->stack_);
		g_top_manager->processing_callbacks_ = true;
	}

	while (!tasks.empty()) {
		base::Closure task = tasks.top();
		task.Run();
		tasks.pop();
	}

	
	DCHECK(g_top_manager->stack_.empty());
}

void AtExitManager::DisableAllAtExitManagers() {
	std::lock_guard<std::mutex> lock(g_top_manager->mutex_);
	g_disable_manager = true;
}

AtExitManager::AtExitManager(bool shadow) 
	: processing_callbacks_(false), next_manager_(g_top_manager){
	DCHECK(shadow || !g_top_manager);
	g_top_manager = this;
}

}	// namespace base.