#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <optional>

template<typename Resource>
class ResourceManager {
public:
    void addResources(std::vector<std::unique_ptr<Resource>> resources)
    {
        for (auto& resource : resources)
        {
            m_resources[resource->getName()] = std::move(resource);
        }
    }

    std::optional<std::reference_wrapper<Resource>> getResource(const std::string& name)
    {
        auto it = m_resources.find(name);
        if (it == m_resources.end())
        {
            return std::nullopt;
        }
        return std::reference_wrapper<Resource>(*(it->second));
    }

    void deleteAllResources()
    {
        for (auto& [name, resource] : m_resources)
        {
            resource->destroy();
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Resource>> m_resources;
};
