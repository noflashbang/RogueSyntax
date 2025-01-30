#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <memory>

#include "UIConfig.h"

//simple event layer
template <class>
class UIEventObserver;

template <class T>
class UIEventAgent
{
public:
	
	~UIEventAgent() = default;
	void SetEventData(T data) { _eventData = data; Notify(); }
	T GetEventData() { return _eventData; }

	std::shared_ptr<UIEventObserver<T>> Subscribe()
	{
		auto observer = std::make_shared<UIEventObserver<T>>(this);
		_subscribers.push_back(observer);
		return observer;
	};
	void Unsubscribe(std::shared_ptr<UIEventObserver<T>> observer)
	{
		_subscribers.erase(std::remove_if(_subscribers.begin(), _subscribers.end(), [observer](const std::weak_ptr<UIEventObserver<T>>& item) { return item.lock() == observer; }), _subscribers.end());
	};

private:
	T _eventData;
	void Notify()
	{
		for (auto& subscriber : _subscribers)
		{
			if (auto observer = subscriber.lock())
			{
				observer->Update(_eventData);
			}
		}
	};
	std::vector<std::weak_ptr<UIEventObserver<T>>> _subscribers;
};

template <class T>
class UIEventObserver
{
public:
	UIEventObserver(UIEventAgent<T>* obsThis) : _agent(obsThis) {};
	~UIEventObserver() = default;

	void SetEventData(T data) { _agent->SetEventData(data); }
	T GetEventData() { return _eventData; }

private:
	friend class UIEventAgent<T>; //we do this so that the users of the observers don't call the wrong method
	void Update(T eventData) { _eventData = eventData; }; //make this private so that only the agent can call it
	T _eventData;
	UIEventAgent<T>* _agent;
};