#pragma once
#include <cstdint>
#include <string>
namespace Loong::Resource {

class LoongRuntimeShaderCode {
public:
	std::string vertexShader;
	std::string fragmentShader;
};

class LoongRuntimeShader {
public:
	void SetUseAoMap(bool b) { if (b) { defMask_ |= (1u<<0u); } else { defMask_ &= ~(1u<<0u); } }
	bool IsUseAoMap() const { return defMask_ & (1u<<0u); }

	void SetUseMatallicMap(bool b) { if (b) { defMask_ |= (1u<<1u); } else { defMask_ &= ~(1u<<1u); } }
	bool IsUseMatallicMap() const { return defMask_ & (1u<<1u); }

	void SetUseEmissiveMap(bool b) { if (b) { defMask_ |= (1u<<2u); } else { defMask_ &= ~(1u<<2u); } }
	bool IsUseEmissiveMap() const { return defMask_ & (1u<<2u); }

	void SetUseEmissive(bool b) { if (b) { defMask_ |= (1u<<3u); } else { defMask_ &= ~(1u<<3u); } }
	bool IsUseEmissive() const { return defMask_ & (1u<<3u); }

	void SetUseNormalMap(bool b) { if (b) { defMask_ |= (1u<<4u); } else { defMask_ &= ~(1u<<4u); } }
	bool IsUseNormalMap() const { return defMask_ & (1u<<4u); }

	void SetUseAlbedoMap(bool b) { if (b) { defMask_ |= (1u<<5u); } else { defMask_ &= ~(1u<<5u); } }
	bool IsUseAlbedoMap() const { return defMask_ & (1u<<5u); }

	void SetUseRoughnessMap(bool b) { if (b) { defMask_ |= (1u<<6u); } else { defMask_ &= ~(1u<<6u); } }
	bool IsUseRoughnessMap() const { return defMask_ & (1u<<6u); }

	uint32_t GetDefinitionMask() const { return defMask_; }
	LoongRuntimeShaderCode GenerateShaderSources() const;
private:
	uint32_t defMask_{0};
};

inline bool operator<(const LoongRuntimeShader& a, const LoongRuntimeShader& b) {
	 return a.GetDefinitionMask() < b.GetDefinitionMask();
}

}
