#include <algorithm>
#include <iostream>

#include "EntityManager.h"
#include "TextureManager.h"

bool EntityManager::_checkInstanceIDValid(InstanceID instanceID) const {
    if (instanceID.slot >= _entities.size() || !_entities[instanceID.slot]) {
        LOG_I << "Entity instance in slot " << instanceID.slot << " not found." << std::endl;
        return false;
    }
    if (_entityGenerations[instanceID.slot] != instanceID.generation) {
        LOG_I << "Entity instance in slot " << instanceID.slot << " has a different generation." << std::endl;
        return false;
    }
    return true;
}

size_t EntityManager::AddEntityDefinition(EntityDefinition&& definition) {
    auto it = _entityNameToID.find(definition.name);
    if (it != _entityNameToID.end()) {
        LOG_W << "Entity definition with name " << definition.name << " already exists." << std::endl;
        return it->second;
    }
    _entityDefinitions.push_back(std::make_unique<EntityDefinition>(std::move(definition)));
    EntityDefinition* defPtr = _entityDefinitions.back().get();
    size_t id = _entityDefinitions.size() - 1;
    _entityNameToID[_entityDefinitions[id]->name] = id;
    _entityIDToInstanceIDs.emplace_back();
    std::string entityType = defPtr->type;
    if (_palette.find(entityType) == _palette.end()) {
        _palette[entityType] = {};
    }
    _palette[entityType].push_back({defPtr->name, id, defPtr->GetRepresentativeTexture()});
    return id;
}

const std::vector<const EntityDefinition*> EntityManager::GetEntityDefinitions() const {
    std::vector<const EntityDefinition*> definitions;
    for (const auto& defPtr : _entityDefinitions) {
        definitions.push_back(defPtr.get());
    }
    return definitions;
}

std::optional<size_t> EntityManager::GetMaxEntityID() const {
    if (_entityDefinitions.empty()) {
        return std::nullopt;
    }
    return _entityDefinitions.size() - 1;
}

std::optional<size_t> EntityManager::GetEntityDefinitionID(const std::string& entityName) const {
    auto it = _entityNameToID.find(entityName);
    if (it == _entityNameToID.end()) {
        LOG_I << "Entity definition with name " << entityName << " not found." << std::endl;
        return std::nullopt;
    }
    return it->second;
}

const EntityDefinition* EntityManager::GetEntityDefinition(const size_t entityID) const {
    if (entityID >= _entityDefinitions.size()) {
        LOG_I << "Entity definition with ID " << entityID << " not found." << std::endl;
        return nullptr;
    }
    return _entityDefinitions[entityID].get();
}

void EntityManager::RemoveAllEntityDefinitions() {
    RemoveAllEntityInstances();
    _entityDefinitions.clear();
    _entityNameToID.clear();
}

std::optional<InstanceID> EntityManager::CreateEntityInstance(size_t entityID, const PropertyValueVect& properties) {
    auto* entityDef = GetEntityDefinition(entityID);
    if (!entityDef) {
        LOG_I << "Failed to create entity instance.";
        return std::nullopt;
    }
    InstanceID instanceID;
    EntityInstance* instance;
    
    if (!_entitySlots.empty()) {
        instanceID.slot = _entitySlots.back();
        _entitySlots.pop_back();
        instanceID.generation = ++_entityGenerations[instanceID.slot];
        _entities[instanceID.slot] = std::make_unique<EntityInstance>(entityID, *this);
        instance = _entities[instanceID.slot].get();
    } else {
        instanceID.slot = _entities.size();
        instanceID.generation = 0;
        _entityGenerations.push_back(0);
        _entities.emplace_back(std::make_unique<EntityInstance>(entityID, *this));
        instance = _entities.back().get();
    }
    if (entityID >= _entityIDToInstanceIDs.size()) {
        _entityIDToInstanceIDs.resize(entityID + 1);
    }
    _entityIDToInstanceIDs[entityID].push_back(instanceID);
    if (!instance->ApplyProperties(properties)) {
        RemoveEntityInstance(instanceID);
        return std::nullopt;
    }
    return instanceID;
}

std::vector<const EntityInstance*> EntityManager::GetEntityInstances(const std::optional<InstanceID> ignoredID) const {
    std::vector<const EntityInstance*> instances;
    for (size_t pos = 0; pos < _entities.size(); pos++)
    {
        if (_entities[pos] && (!ignoredID || *ignoredID != InstanceID{pos, _entityGenerations[pos]})) {
            instances.push_back(_entities[pos].get());
        }
    }
    return instances;
}

std::vector<InstanceID> EntityManager::GetEntityInstanceIDs(const std::optional<InstanceID> ignoredID) const {
    std::vector<InstanceID> instanceIDs;
    for (size_t pos = 0; pos < _entities.size(); pos++)
    {
        if (_entities[pos] && (!ignoredID || *ignoredID != InstanceID{pos, _entityGenerations[pos]})) {
            instanceIDs.push_back(InstanceID{pos, _entityGenerations[pos]});
        }
    }
    return instanceIDs;
}

std::vector<const EntityInstance*> EntityManager::GetEntityInstancesByID(size_t entityID, std::optional<InstanceID> ignoredID) const {
    std::vector<const EntityInstance*> instances;
    if (entityID >= _entityIDToInstanceIDs.size()) {
        LOG_I << "Entity definition with ID " << entityID << " not found." << std::endl;
        return instances;
    }
    for (const auto& instanceID : _entityIDToInstanceIDs[entityID])
    {
        if (!ignoredID || *ignoredID != instanceID) {
            auto* instance = GetEntityInstance(instanceID);
            if (instance) {
                instances.push_back(instance);
            }
        }
    }
    return instances;
}

EntityInstance* EntityManager::_getEntityInstance(InstanceID instanceID) const {
    if (!_checkInstanceIDValid(instanceID)) {
        return nullptr;
    }
    return _entities[instanceID.slot].get();
}

EntityInstance* EntityManager::GetEntityInstance(InstanceID instanceID, std::optional<InstanceID> ignoredID) {
    if (ignoredID && *ignoredID == instanceID) {
        return nullptr;
    }
    return _getEntityInstance(instanceID);
}

const EntityInstance* EntityManager::GetEntityInstance(InstanceID instanceID, std::optional<InstanceID> ignoredID) const {
    if (ignoredID && *ignoredID == instanceID) {
        return nullptr;
    }
    return _getEntityInstance(instanceID);
}

void EntityManager::RemoveEntityInstance(InstanceID instanceID) {
    if (!_checkInstanceIDValid(instanceID)) {
        LOG_I << "Failed to remove entity instance (" << instanceID.slot << ":" << instanceID.generation << " not found)" << std::endl;
        return;
    }
    size_t entityID = _entities[instanceID.slot]->entityID;
    _entities[instanceID.slot].reset();
    _entitySlots.push_back(instanceID.slot);

    auto& instanceIDs = _entityIDToInstanceIDs[entityID];
    instanceIDs.erase(std::remove(instanceIDs.begin(), instanceIDs.end(), instanceID), instanceIDs.end());
}

void EntityManager::RemoveAllEntityInstances() {
    for (size_t i = 0; i < _entities.size(); i++) {
        if (_entities[i]) {
            _entities[i].reset();
            _entitySlots.push_back(i);
        }
    }
    for (auto& instanceIDs : _entityIDToInstanceIDs) {
        instanceIDs.clear();
    }
}

const SpriteCollection& EntityManager::GetSpriteCollection(size_t entityID) const {
    auto* entityDef = GetEntityDefinition(entityID);
    if (!entityDef) {
        LOG_I<< "Failed to get SpriteCollection: Entity definition with ID " << entityID << " not found." << std::endl;
        return _textureManager.GetCollection(0);
    }
    return _textureManager.GetCollection(entityDef->GetSpriteCollection());
}

const sf::Texture& EntityManager::GetRepresentativeTexture(size_t entityID) const {
    auto* entityDef = GetEntityDefinition(entityID);
    if (!entityDef) {
        LOG_I << "Failed to get representative texture: Entity definition with ID " << entityID << " not found." << std::endl;
        return _textureManager.GetTexture(0);
    }
    return _textureManager.GetTexture(entityDef->GetRepresentativeTexture());
}

bool EntityManager::RestoreEntityInstance(InstanceID instanceID, size_t entityID, const PropertyValueVect& properties) {
    if (instanceID.slot < _entities.size() && _entities[instanceID.slot] != nullptr) {
        LOG_W << "Cannot restore entity: slot " << instanceID.slot << " is already occupied." << std::endl;
        return false;
    }

    auto* entityDef = GetEntityDefinition(entityID);
    if (!entityDef) {
        LOG_I << "Failed to restore entity instance: Definition not found.";
        return false;
    }
    auto slotIt = std::find(_entitySlots.begin(), _entitySlots.end(), instanceID.slot);
    if (slotIt != _entitySlots.end()) {
        _entitySlots.erase(slotIt);
    }

    if (instanceID.slot >= _entities.size()) {
        _entities.resize(instanceID.slot + 1);
        _entityGenerations.resize(instanceID.slot + 1, 0);
    }

    _entityGenerations[instanceID.slot] = instanceID.generation;
    _entities[instanceID.slot] = std::make_unique<EntityInstance>(entityID, *this);
    auto* instance = _entities[instanceID.slot].get();

    if (entityID >= _entityIDToInstanceIDs.size()) {
        _entityIDToInstanceIDs.resize(entityID + 1);
    }
    _entityIDToInstanceIDs[entityID].push_back(instanceID);

    if (!instance->ApplyProperties(properties)) {
        LOG_I << "Failed to apply properties during entity restoration.";
        RemoveEntityInstance(instanceID);
        return false;
    }

    return true;
}
