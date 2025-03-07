#include "beans.hpp"

namespace beans
{

namespace error
{

InterfaceNotDeclaredError::InterfaceNotDeclaredError(const std::type_info& interface)
        : m_typeInfo{&interface}
        , m_what{std::string{"Beans: Implementation for \""} + interface.name() + "\" was not declared."}
{
}
InterfaceNotDeclaredError::InterfaceNotDeclaredError(const std::type_info& interface, const std::string& tag)
        : m_typeInfo{&interface}
        , m_tag{tag}
        , m_what{std::string{"Beans: Implementation for \""} + interface.name() + "\" with tag \""
                             + tag + "\" was not declared."}
{
}

const char *InterfaceNotDeclaredError::what() const noexcept
{
    return m_what.c_str();
}

const std::type_info &InterfaceNotDeclaredError::getTypeInfo() const
{
    return *m_typeInfo;
}

} // namespace error


namespace internal
{

TypeDatabaseManager::TypeDatabaseManager() {}

const TypeInformation* TypeDatabaseManager::shallowFind(
        const std::type_info* interface,
        const std::string& tag) const
{

    auto it = m_db.find(interface);
    if (it == m_db.end())
        return nullptr;

    TypeInformation const * match = nullptr;

    // If tag is specified, search an implementation with the tag (start from the last one)
    if (tag.empty())
    {
        for (auto it2 = it->second.rbegin(); it2 != it->second.rend(); ++it2)
        {
            if (tag == it2->tag)
            {
                match = &*it2;
                break;
            }
        }
    }
    // Otherwise... 
    else
    {
        // ...first try to find an implementation with empty tag (start from the last one)
        for (auto it2 = it->second.rbegin(); it2 != it->second.rend(); ++it2)
        {
            if (it2->tag.empty())
            {
                match = &*it2;
                break;
            }
        }
        // ...if not found, get any implementation (start from the last one)
        if (nullptr == match && !it->second.empty())
            match = &it->second.back();
    }

    return match;
}

const TypeInformation *TypeDatabaseManager::deepFind(
        const std::type_info* interface,
        const std::string &tag) const
{
    if (nullptr != m_child)
    {
        auto* match = m_child->deepFind(interface, tag);
        if (nullptr != match)
            return match;
    }

    auto* match = shallowFind(interface, tag);
    return match;
}

const TypeDatabaseManager *TypeDatabaseManager::getChild() const
{
    return m_child.get();
}

TypeDatabaseManager* TypeDatabaseManager::getChild()
{
    return m_child.get();
}

void TypeDatabaseManager::setChild(TypeDatabaseManager* child)
{
    m_child.reset(child);
}

TypeDatabaseManager* TypeDatabaseManager::getParent()
{
    return m_parent;
}

void TypeDatabaseManager::setParent(TypeDatabaseManager* parent)
{
    m_parent = parent;
}

Mutex& getMutex()
{
    static Mutex s_mx;
    return s_mx;
}

TypeDatabaseManager& topLevelDb()
{
    static TypeDatabaseManager s_topLevelDb;
    return s_topLevelDb;
}

TypeDatabaseManager& lowLevelDb()
{
    auto* db = &topLevelDb();
    while (db->getChild())
        db = db->getChild();
    return *db;
}

} // namespace internal

} // namespace beans