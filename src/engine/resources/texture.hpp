#pragma once

#include <stb_image.h>

#include "../resource.hpp"

// AUTHOR	:	Aislan A
// PURPOSE	:	bro...

class Texture : public Resource {
public:
	Texture() : Resource() {};
	~Texture() : ~Resource() {};

	void load() override;
	void unload() override;

	void bind(unsigned int slot = 0);
private:
};