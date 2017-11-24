#pragma once
#include "Common.h"
#include "Version.h"
#include "AssetController.h"

#include "NetLayer.h"

#include "Level.h"
#include "Actor.h"
#include "Object.h"

#include "PlayerController.h"

#include <vector>
#include <unordered_map>


class Engine;


/**
* Contains any relevent information about a given game
* e.g. Assets to load, Supported actor types, Supported levels etc.
*/
class CORE_API Game
{
private:
	friend NetSession;
	string m_name;
	Engine* m_engine = nullptr;
	Version m_version;

	AssetController m_assetController;
	LLevel* m_currentLevel = nullptr;

	std::unordered_map<uint16, SubClassOf<LLevel>> m_registeredLevels;
	std::unordered_map<uint16, SubClassOf<OObject>> m_registeredObjectTypes;
	std::unordered_map<uint16, SubClassOf<AActor>> m_registeredActorTypes;

	std::vector<OObject*> m_activeObjects;
	std::unordered_map<uint16, OObject*> m_netObjectLookup;

public:
	/// Level to load at start (For client)
	SubClassOf<LLevel> defaultLevel;
	/// Level to load at start (For server)
	SubClassOf<LLevel> defaultNetLevel;

	/// Class type to use when creating a net layer
	SubClassOf<NetLayer> netLayerClass;
	/// Class type to use for any player connections
	SubClassOf<OPlayerController> playerControllerClass;

public:
	Game(string name, Version version);
	~Game();

	/**
	* Callback for when the engine prepares to launch the game
	* @param engine		Engine in question
	*/
	void OnGameHooked(Engine* engine);

	/**
	* Callback from engine for every tick by main loop
	* @param deltaTime		Time since last update (In seconds)
	*/
	void MainUpdate(const float& deltaTime);

#ifdef BUILD_CLIENT
	/**
	* Callback from engine for every tick by display
	* @param deltaTime		Time since last update (In seconds)
	*/
	void DisplayUpdate(const float& deltaTime);
#endif

	/**
	* Registers a given class, so it may be loaded by the game
	* Usage should be with subclasses of Objects, Actors or Levels
	* @param classType			The class of the type to register
	*/
	void RegisterClass(const MClass* classType);

	/**
	* Is there a registered object with this id
	* @param id				The id to lookup
	* @returns If the game has a registered object type using that id
	*/
	inline bool IsRegisteredObject(const uint16& id) const { return m_registeredObjectTypes.find(id) != m_registeredObjectTypes.end(); }
	/**
	* Is there a registered actor with this id
	* @param id				The id to lookup
	* @returns If the game has a registered actor type using that id
	*/
	inline bool IsRegisteredActor(const uint16& id) const { return m_registeredActorTypes.find(id) != m_registeredActorTypes.end(); }
	/**
	* Is there a registered level with this id
	* @param id				The id to lookup
	* @returns If the game has a registered level using that id
	*/
	inline bool IsRegisteredLevel(const uint16& id) const { return m_registeredLevels.find(id) != m_registeredLevels.end(); }

	/**
	* Retrieve object class from it's id
	* @param id			The id of the object
	* @returns The class, if found or nullptr if failed
	*/
	inline const MClass* GetObjectClass(const uint16& id) const {
		auto it = m_registeredObjectTypes.find(id);
		return it == m_registeredObjectTypes.end() ? nullptr : *(it->second);
	}
	/**
	* Retrieve actor class from it's id
	* @param id			The id of the actor
	* @returns The class, if found or nullptr if failed
	*/
	inline const MClass* GetActorClass(const uint16& id) const {
		auto it = m_registeredActorTypes.find(id);
		return it == m_registeredActorTypes.end() ? nullptr : *(it->second);
	}
	/**
	* Retrieve level class from it's id
	* @param id			The id of the level
	* @returns The class, if found or nullptr if failed
	*/
	inline const MClass* GetLevelClass(const uint16& id) const {
		auto it = m_registeredLevels.find(id);
		return it == m_registeredLevels.end() ? nullptr : *(it->second);
	}


	/**
	* Switch level to this class (If registered)
	* @param levelType			The class of the level to switch to
	* @returns True if the level switches
	*/
	bool SwitchLevel(const SubClassOf<LLevel>& levelType);
	/**
	* Switch level to this id (If registered)
	* @param levelId			The class of the level to switch to
	* @returns True if the level switches
	*/
	bool SwitchLevel(const uint16& levelId);


	/**
	* Add an object to the game (Forfeits memory rights to level)
	* @param object		The object to add
	*/
	void AddObject(OObject* object);

	/**
	* Spawns an object of the given type
	* @param objectClass		The class of the object to spawn
	* @param owner				The object who is seen as this object's owner
	* @returns New actor object or nullptr, if invalid
	*/
	template<class ObjectType>
	ObjectType* SpawnObject(const SubClassOf<ObjectType>& objectClass = ObjectType::StaticClass(), const OObject* owner = nullptr) { return static_cast<ObjectType*>(SpawnObject<OObject>(*objectClass, owner)); }
	/**
	* Spawns an object of the given type
	* @param objectClass		The class of the object to spawn
	* @param owner				The object who is seen as this object's owner
	* @returns New object or nullptr, if invalid
	*/
	template<>
	OObject* SpawnObject(const SubClassOf<OObject>& objectClass, const OObject* owner);


	/**
	* Getters & Setters
	*/
public:
	inline Engine* GetEngine() const  { return m_engine; }
	inline LLevel* GetCurrentLevel() const { return m_currentLevel; }

	inline string GetName() const { return m_name; }
	inline const Version& GetVersionNo() const { return m_version; }

	inline AssetController* GetAssetController() { return &m_assetController; }

	NetSession* GetSession() const;

	/**
	* Returns all active objects
	*/
	inline const std::vector<OObject*>& GetActiveObjects() const { return m_activeObjects; }

	/**
	* Return all active objects of this class
	* @param type			The class type to query for
	*/
	inline const std::vector<OObject*> GetActiveObjects(const MClass* type) const
	{
		std::vector<OObject*> output;
		for (uint32 i = 0; i < m_activeObjects.size(); ++i)
			if (m_activeObjects[i]->GetClass()->IsChildOf(type))
				output.emplace_back(m_activeObjects[i]);
		return output;
	}

	/**
	* Return all active objects of this class type
	*/
	template<class ObjectType>
	inline const std::vector<ObjectType*> GetActiveObjects() const
	{
		std::vector<ObjectType*> output;
		for (uint32 i = 0; i < m_activeObjects.size(); ++i)
		{
			ObjectType* casted = dynamic_cast<ObjectType*>(m_activeObjects[i]);
			if (casted != nullptr)
				output.emplace_back(casted);
		}
		return output;
	}


	/**
	* Retrieve an object from it's network id
	* @param id			Network id of this object
	* @returns The object of this id or nullptr, if not found
	*/
	OObject* GetObjectByNetID(const uint32& id) const;
};