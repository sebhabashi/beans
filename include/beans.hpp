#pragma once

#include <stdexcept>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>

#define BEANS_INTERNAL_STATIC_TYPES_CHECKS(interface, implementation) \
    static_assert(std::is_class<interface>::value, "Interface must be a class type."); \
    static_assert(std::is_class<implementation>::value, "Implementation must be a class type."); \
    static_assert(!std::is_same<interface, implementation>::value, \
                  "Interface and implementation cannot be the same type."); \
    static_assert(std::is_assignable<interface*&, implementation*>::value, \
                    "Implementation does not implement interface.");

namespace beans
{

namespace error
{

/**
 * 
 */

/// @brief Error caused when a bean is created but no implementation nor instance was registered
/// for its interface and tag
class InterfaceNotDeclaredError : public std::exception
{
public:
    /// @brief Create an error for an unregistered interface without tag
    InterfaceNotDeclaredError(const std::type_info& interface);

    /// @brief Create an error for an unregistered interface with a tag
    InterfaceNotDeclaredError(const std::type_info& interface, const std::string& tag);

    /// @brief Get the error text
    /// @return The error text
    const char* what() const noexcept;

    /// @brief Get the information about the unregistered interface
    /// @return The information about the unregistered interface
    const std::type_info& getTypeInfo() const;
private:

    /// Information of the interface
    const std::type_info* m_typeInfo;

    /// Tag
    std::string m_tag;

    /// Error text
    std::string m_what;
};

} // namespace error

/// @brief Internal utilities
namespace internal
{

/// @brief Simple class that executes a function at construction
class DoAtStart
{
public:
    DoAtStart(const std::function<void()>& fc) { fc(); }
};

/// Registered information about a type
struct TypeInformation
{
    const std::type_info* typeId;
    enum OwnerShip { OS_OWNED, OS_FROM_INSTANCE } ownerShip;
    std::string shortName;
    std::string qualifName;
    std::string tag;
    std::function<void*()> constructor;
};

/// @brief A database holding type information
class TypeDatabaseManager
{
public:
    /// @brief Construct a database
    TypeDatabaseManager();

    template<typename T>
    T* construct(const std::string& tag = "") const
    {
        return reinterpret_cast<T*>(constructAsVoidPtr(&typeid(T), tag));
    }

    /// @brief Search implementations for an interface and a tag only in this database
    /// @param interface Interface
    /// @param tag Tag (empty means no tag)
    /// @return The corresponding type information
    const TypeInformation* shallowFind(const std::type_info* interface, const std::string& tag) const;

    /// @brief Search implementations for an interface and a tag, in this database and all children
    /// databases
    /// @details The lowest level child will be searched first
    /// @param interface Interface
    /// @param tag Tag (empty means no tag)
    /// @return The corresponding type information
    const TypeInformation* deepFind(const std::type_info* interface, const std::string& tag) const;

    /// @brief Get the child database
    /// @return The child database
    const TypeDatabaseManager* getChild() const;

    /// @brief Get the child database
    /// @return The child database
    TypeDatabaseManager* getChild();

    /// @brief Set the child database
    /// @param child The new child database
    void setChild(TypeDatabaseManager* child);

    /// @brief Get the parent database
    /// @return The parent database
    TypeDatabaseManager* getParent();

    /// @brief Set the parent database
    /// @param parent The new parent database
    void setParent(TypeDatabaseManager* parent);

    /// @brief Register an implementation for the interface
    /// @tparam Interface Interface
    /// @tparam Implementation Implementation of the interface
    /// @param tag Tag for the registration
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

    /// @brief Register an instance for the interface
    /// @tparam Interface Interface
    /// @param tag Tag for the registration
    template<typename Interface>
    void registerInstance(Interface* instance, const std::string& tag)
    {
        auto* interface = &typeid(Interface);
        if (!m_db.count(interface))
            m_db.emplace(interface, std::vector<TypeInformation>());
        auto& implementations = m_db.at(interface);

        TypeInformation info;
        info.shortName = typeid(Interface).name();
        info.qualifName = typeid(Interface).raw_name();
        info.typeId = &typeid(Interface);
        info.ownerShip = TypeInformation::OS_FROM_INSTANCE;
        info.constructor = [instance]() { return instance; };
        implementations.emplace_back(info);
    }

private:
    using Map = std::map<const std::type_info*,         // Interface
                         std::vector<TypeInformation>>; // Implementations

    /// Map containing the registered types
    Map m_db;

    /// Pointer to the parent database 
    TypeDatabaseManager* m_parent = nullptr;

    /// Pointer to the child database
    std::unique_ptr<TypeDatabaseManager> m_child;
};

/// Mutex type
using Mutex = std::recursive_mutex;

/// Mutex lock type
using Lock = std::lock_guard<Mutex>;

/// @brief Get the mutex protecting the databases
Mutex& getMutex();

/// @brief Get the top level database
TypeDatabaseManager& topLevelDb();

/// @brief Get the lowest level database
TypeDatabaseManager& lowLevelDb();

} // namespace internal


/**
 * @brief Abstraction for an implemented interface.
 * 
 * Use this in your parent classes to reference subcomponent classes. If it is registered (using
 * registerImplementation or registerInterface), beans will automatically instantiate the
 * corresponding implementation.
 * 
 * @tparam T: Interface type.
 */
template<typename T>
class Bean
{
public:
    /**
     * Construct a Bean without a tag
     */
    Bean() : Bean("")
    {
    }
    /**
     * Construct a Bean with a tag
     * @param tag: (Optional) Add a tag to the implementation, in case you want to register multiple
     * implementations for the same interface
     */
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
    /// Not used outside of construction/destruction
    /// If the instance is owned by the bean, this points to the same address as m_ptr
    /// Otherwise, it points to nullptr
    /// Because this is defined before m_ptr, it will be deleted after, ensuring m_ptr's defined-ness
    std::shared_ptr<T> m_ownedPtr;

    /// Points to the address of the implementation, whether owned in m_ownedPtr in the case of a
    /// registerInstance, or the unique instance defined with registerInstance
    T* m_ptr{nullptr};
};

/**
 * @brief An atomic environment, where changes to the beans database do not clash with those made
 * in other threads.
 * 
 * Changes made here are lost upon destruction or call to `unlock()` and do not affect the database
 * outside.
 * 
 * Locked environments can be nested. If you register different implementations for the same
 * interface in different environments, the innermost environment has priority.
 * 
 * Once all the beans have been constructed, the environment can be destroyed or unlocked without
 * affecting the beans.
 */
class LockedEnvironment
{
public:
    LockedEnvironment() : m_lock(new internal::Lock(internal::getMutex()))
    {
        auto* child = new internal::TypeDatabaseManager;
        auto& current = internal::lowLevelDb();
        child->setParent(&current);
        current.setChild(child);
        unlocked = false;
    }
    ~LockedEnvironment()
    {
        unlock();
    }

    void unlock()
    {
        if (!unlocked)
        {
            auto& current = internal::lowLevelDb();
            auto* parent = current.getParent();
            parent->setChild(nullptr);
            m_lock.reset();
            unlocked = true;
        }
    }
private:
    std::unique_ptr<internal::Lock> m_lock;
    std::atomic<bool> unlocked;
};

/**
 * Register an implementation for an interface
 * @tparam Interface: Interface
 * @tparam Implementation: Implementation for the interface
 * @param tag: (Optional) Add a tag to the implementation, in case you want to register multiple
 * implementations for the same interface
 */
template<typename Interface, typename Implementation>
void registerImplementation(const std::string& tag = "")
{
    BEANS_INTERNAL_STATIC_TYPES_CHECKS(Interface, Implementation)
    internal::Lock lock(internal::getMutex());
    internal::lowLevelDb().registerImplementation<Interface, Implementation>(tag);
}

/**
 * Register a unique instance for an interface
 * 
 * @details
 * Beans cannot protect the internal data of the instance against thread concurrency races, so the
 * user must handle the protection of this internal data (with a mutex, atomic values etc.).
 * 
 * Beans does not own the instance, so its deletion is up to the user. This deletion should happen
 * after all beans using this instance are destroyed.
 * 
 * @tparam Interface: Interface
 * @param instance: Unique instance, that will be used by all beans for the interface.
 * 
 * @param tag: (Optional) Add a tag to the implementation, in case you want to register multiple
 * implementations for the same interface
 */
template<typename Interface>
void registerInstance(Interface* instance, const std::string& tag = "")
{
    internal::Lock lock(internal::getMutex());
    internal::lowLevelDb().registerInstance<Interface>(instance, tag);
}

} // namespace beans

/// Declare an instance as the default implementation for an interface globally
/// @param interface The interface
/// @param implementation Implementation for the interface
#define BEANS_DEFAULT_IMPLEMENTATION(interface, implementation) \
        /* Forward-declare interface and implementation */ \
        class interface; \
        class implementation; \
        \
        namespace beans \
        { \
            namespace internal \
            { \
                /* Register the implementation at start of program, using a global dummy variable */ \
                DoAtStart register_##implementation##_For_##interface([] \
                { \
                    beans::registerImplementation<interface, implementation>(); \
                }); \
            } /* namespace internal */ \
        } /* namespace beans */

