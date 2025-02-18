#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <atomic>


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
	T& GetEventData() { return _eventData; }

private:
	friend class UIEventAgent<T>; //we do this so that the users of the observers don't call the wrong method
	void Update(T eventData) { _eventData = eventData; }; //make this private so that only the agent can call it
	T _eventData;
	UIEventAgent<T>* _agent;
};


// Generic Lockable Concept
template <typename LockType>
concept Lockable = requires(LockType lock)
{
	lock.lock();
	lock.unlock();
};

// No-op lock (for single-threaded or lock-free cases)
class NoLock
{
public:
	void lock() {}
	void unlock() {}
};

// Spinlock implementation (lightweight, good for low contention)
class SpinLock
{
public:
	void lock()
	{
		while (_flag.test_and_set(std::memory_order_acquire));
	}
	void unlock() 
	{
		_flag.clear(std::memory_order_release);
	}
private:
	std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};

class ScopedConnection 
{
public:
	using DisconnectFn = std::function<void()>;

	ScopedConnection() : _disconnect(nullptr), _delayedUnsubscribeGuard()
	{
	};

	explicit ScopedConnection(DisconnectFn disconnect, std::weak_ptr<size_t> parentGuard) : _disconnect(std::move(disconnect)), _delayedUnsubscribeGuard(parentGuard)
	{
	};

	~ScopedConnection() 
	{ 
		if (_disconnect != nullptr) 
		{ 
			if (auto guard = _delayedUnsubscribeGuard.lock())
			{
				_disconnect();
			}
		} 
	};

private:
	DisconnectFn _disconnect;
	std::weak_ptr<size_t> _delayedUnsubscribeGuard;
};


template<typename mutex_type, typename... Args>
requires Lockable<mutex_type>
class UI_Delegate
{
public:
	using HandlerType = std::function<void(Args...)>;

	class Connection
	{
	public:
		Connection(UI_Delegate& event, size_t id) : _event(event), _id(id)
		{
		}
		~Connection()
		{
			Disconnect();
		}
		void Disconnect()
		{
			if (_connected)
			{
				_event.Unsubscribe(_id);
				_connected = false;
			}
		};

	private:
		bool _connected = true;
		UI_Delegate& _event;
		size_t _id;
	};

	explicit UI_Delegate() : _lockPolicy(std::make_unique<NoLock>())
	{
	};

	explicit UI_Delegate(std::unique_ptr<mutex_type> lockPolicy) : _lockPolicy(std::move(lockPolicy))
	{
	};

	
	[[nodiscard]] std::unique_ptr<ScopedConnection> operator+=(HandlerType handler)
	{
		std::lock_guard lock(*_lockPolicy);
		auto id = _nextId++;
		_handlers[id] = std::move(handler);
		return std::make_unique<ScopedConnection>([this, id]() { Unsubscribe(id); }, _delayedUnsubscribeGuard);
	};

	template <typename T>
	[[nodiscard]] std::unique_ptr<ScopedConnection> Bind(std::weak_ptr<T> weakObj, void (T::* method)(Args...))
	{
		return *this 
			+= [weakObj, method](Args... args) 
				{
					if (auto obj = weakObj.lock()) 
					{  
						// Ensure object is still alive
						(obj.get()->*method)(args...);
					}
				};
	}

	void operator-=(const std::shared_ptr<Connection>& connection)
	{
		if (connection)
		{
			std::lock_guard lock(*_lockPolicy);
			Unsubscribe(connection->_id);
		}
	};

	void operator()(Args... args) const
	{
		std::lock_guard lock(*_lockPolicy);
		for (const auto& [id, handler] : _handlers)
		{
			handler(args...);
		}
	};

private:
	void Unsubscribe(size_t id)
	{
		_handlers.erase(id);
	};

	mutable std::unordered_map<size_t, HandlerType> _handlers;
	size_t _nextId = 0;
	std::unique_ptr<mutex_type> _lockPolicy;
	std::shared_ptr<size_t> _delayedUnsubscribeGuard = std::make_unique<size_t>(0);
};

template<typename mutex_type, typename T>
	requires Lockable<mutex_type>
class Bindable 
{
public:
	using CallbackType = std::function<void(const T&)>;

	explicit Bindable(T value) : _value(std::move(value)), _changeEvent(UI_Delegate<mutex_type, const T&>())
	{
	};

	explicit Bindable(T value, std::unique_ptr<mutex_type> lockPolicy) : _value(std::move(value)), _changeEvent(UI_Delegate<mutex_type, const T&>(std::move(lockPolicy)))
	{
	};

	void set(const T& newValue)
	{
		if (_value != newValue)
		{
			_value = newValue;
			notify();
		}
	};
	const T& get() const { return _value; }
	UI_Delegate<mutex_type, const T&>& onChange() { return _changeEvent; }

private:
	void notify()
	{
		_changeEvent(_value);
	};

	T _value;
	UI_Delegate<mutex_type, const T&> _changeEvent;
};
