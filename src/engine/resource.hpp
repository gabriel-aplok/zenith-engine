#pragma once

// AUTHOR	:	Aislan A
// PURPOSE	:	Used as a base to other asset types Ex: Textures, Models, etc.

#include <iostream>
#include <string>

enum ResourceType {
	TEXTURE,
	MODEL,
	AUDIO,
	NONE // ERROR
};

class Resource {
public:
	Resource(ResourceType type);
	~Resource();

	virtual void load();
	virtual void unload();

	// Callbacks
	std::string getAssetName() const { return m_resource_name; };
	int32_t getAssetID() const { return m_resource_id; };
	bool isLoaded() const { return m_loaded; };
private:
	std::string m_resource_name = "";
	int32_t m_resource_id = 0;
	ResourceType m_resource_type = NONE;

	bool m_loaded = false;
};