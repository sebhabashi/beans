#pragma once

#define BEANS_DECLARE_COMPONENT(...)
#define BEANS_DECLARE_INTERFACE(...)

#include <stdexcept>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

namespace beans
{

namespace error
{

class InterfaceNotDeclaredError : public std::exception
{
public:
    InterfaceNotDeclaredError(const std::type_info& interface);
    InterfaceNotDeclaredError(const std::type_info& interface, const std::string& tag);
    const char* what() const noexcept;
private:
    const std::type_info* m_typeInfo;
    std::string m_tag;
    std::string m_what;
};

} // namespace error


namespace internal
{

struct TypeInformation
{
    const std::type_info* typeId;
    enum OwnerShip { OS_OWNED, OS_FROM_INSTANCE } ownerShip;
    std::string shortName;
    std::string qualifName;
    std::string tag;
    std::function<void*()> constructor;
};

class TypeDatabaseManager
{
public:
    TypeDatabaseManager();

    template<typename T>
    T* construct(const std::string& tag = "") const
    {
        return reinterpret_cast<T*>(constructAsVoidPtr(&typeid(T), tag));
    }

    const TypeInformation* shallowFind(const std::type_info* interface, const std::string& tag) const;
    const TypeInformation* deepFind(const std::type_info* interface, const std::string& tag) const;
    const TypeDatabaseManager* getChild() const;
    TypeDatabaseManager* getChild();
    void setChild(TypeDatabaseManager* child);
    TypeDatabaseManager* getParent();
    void setParent(TypeDatabaseManager* parent);

    template<typename Interface, typename Implementation>
    void registerImplementation(const std::string& tag)
    {
        auto* interface = &typeid(Interface);
        if (!m_db.count(interface))
            m_db.emplace(interface, std::vector<TypeInformation>());
        auto& implementations = m_db.at(interface);

        TypeInformation info;
        info.shortName = typeid(Interface).name();
        info.qualifName = typeid(Interface).raw_name();
        info.typeId = &typeid(Implementation);
        info.ownerShip = TypeInformation::OS_OWNED;
        info.constructor = []() { return new Implementation; };
        implementations.emplace_back(info);
    }

    template<typename Interface, typename Implementation>
    void registerInstance(Implementation* instance, const std::string& tag)
    {
        auto* interface = &typeid(Interface);
        if (!m_db.count(interface))
            m_db.emplace(interface, std::vector<TypeInformation>());
        auto& implementations = m_db.at(interface);

        TypeInformation info;
        info.shortName = typeid(Interface).name();
        info.qualifName = typeid(Interface).raw_name();
        info.typeId = &typeid(Implementation);
        info.ownerShip = TypeInformation::OS_FROM_INSTANCE;
        info.constructor = [instance]() { return instance; };
        implementations.emplace_back(info);
    }

private:
    using Map = std::map<const std::type_info*,         // Interface
                         std::vector<TypeInformation>>; // Implementations

    Map m_db;
    TypeDatabaseManager* m_parent = nullptr;
    std::unique_ptr<TypeDatabaseManager> m_child;
};

using Mutex = std::recursive_mutex;
using Lock = std::lock_guard<Mutex>;

Mutex& getMutex();
TypeDatabaseManager& topLevelDb();
TypeDatabaseManager& lowLevelDb();

} // namespace internal


template<typename T>
class Bean
{
public:
    Bean() : Bean("")
    {
    }
    Bean(const std::string& tag)
    {
        internal::Lock lock(internal::getMutex());
        const auto* info = internal::topLevelDb().deepFind(&typeid(T), tag);
        if (nullptr == info)
        {
            if (tag.empty())
                throw error::InterfaceNotDeclaredError(typeid(T));
            else
                throw error::InterfaceNotDeclaredError(typeid(T), tag);
        }

        m_ptr = reinterpret_cast<T*>(info->constructor());
        if (info->ownerShip == internal::TypeInformation::OS_OWNED)
            m_ownedPtr.reset(m_ptr);
    }
    ~Bean()
    {
    }

    T* operator->()
    {
        return m_ptr;
    }
    const T* operator->() const
    {
        return m_ptr;
    }
    T* operator*()
    {
        return m_ptr;
    }
    const T* operator*() const
    {
        return m_ptr;
    }

private:
    T* m_ptr{nullptr};
    std::shared_ptr<T> m_ownedPtr;
};

class LockedEnvironment
{
public:
    LockedEnvironment() : m_lock(new internal::Lock(internal::getMutex()))
    {
        auto* child = new internal::TypeDatabaseManager;
        auto& current = internal::lowLevelDb();
        child->setParent(&current);
        current.setChild(child);
    }
    ~LockedEnvironment()
    {
        auto& current = internal::lowLevelDb();
        auto* parent = current.getParent();
        parent->setChild(nullptr);
    }

    void unlock()
    {
        m_lock.reset();
    }
private:
    std::unique_ptr<internal::Lock> m_lock;
};

template<typename Interface, typename Implementation>
void registerImplementation(const std::string& tag = "")
{
    internal::Lock lock(internal::getMutex());
    internal::lowLevelDb().registerImplementation<Interface, Implementation>(tag);
}

template<typename Interface, typename Implementation>
void registerInstance(Implementation* instance, const std::string& tag = "")
{
    internal::Lock lock(internal::getMutex());
    internal::lowLevelDb().registerInstance<Interface, Implementation>(instance, tag);
}

} // namespace beans