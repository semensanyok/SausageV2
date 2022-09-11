#include "Texture.h"
#include <SystemsManager.h>

using namespace std;

void Texture::MakeHashable(SausageHashable* hash) {
  hash = hash;
  SystemsManager::GetInstance()->texture_manager->AddToLookups(this);
}

/**
* Stores id and handle for texture array.
*/
void Texture::MakeResident() {
	if (is_destoyed) {
		LOG("ERROR making texutre resident. texture is destroyed");
		return;
	}
	if (!is_resident) {
		glMakeTextureHandleResidentARB(texture_handle_ARB);
		is_resident = true;
	}
}
void Texture::MakeNonResident() {
	if (is_resident) {
		glMakeTextureHandleNonResidentARB(texture_handle_ARB);
		is_resident = false;
	}
}
void Texture::BindSingleSampler(unsigned int location) {
	glUniformHandleui64ARB(location, texture_handle_ARB);
}
void Texture::Dispose() {
	MakeNonResident();
	glDeleteTextures(1, &texture_id);
	is_destoyed = true;
}

inline Texture::~Texture() {
  if (!is_destoyed) {
    Dispose();
  };
  SystemsManager::GetInstance()->texture_manager->id_pool->ReleaseNumber(id);
  //delete hash;
}
