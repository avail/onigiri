#pragma once

class RenderState
{
public:
	static inline bool m_enableReflections{true};
	static inline float m_unkFloat1{ 1.0f };
	static inline float m_lodRadius{ 1000.0f };
	static bool SetReflectionEnabled(bool state) { m_enableReflections = state; }
};

class CRenderPhase
{
public:
	virtual void Pad1() = 0;
	virtual void Pad2() = 0;
	virtual void Pad3() = 0;
	virtual void* Render() = 0;

	char gap0[32];
	char char20;
	char gap21[1167];
	char byte4B0;
	char gap4B1;
	char byte4B2;

};