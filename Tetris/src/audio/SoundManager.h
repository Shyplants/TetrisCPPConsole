#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include <fmod.hpp>

struct FmodSystemDeleter
{
	void operator()(FMOD::System* p) const noexcept
	{
		if (!p)
			return;

		p->close();
		p->release();
	}
};

struct FmodSoundDeleter
{
	void operator()(FMOD::Sound* p) const noexcept
	{
		if (!p)
			return;

		p->release();
	}
};

using FmodSystemPtr = std::unique_ptr<FMOD::System, FmodSystemDeleter>;
using FmodSoundPtr = std::unique_ptr<FMOD::Sound, FmodSoundDeleter>;

class SoundManager
{
public:
	SoundManager() = default;
	~SoundManager();

	// FMOD 시스템 초기화 / 해제
	bool Init();
	void Shutdown();

	void Update();
	
	// 로드
	bool LoadBGM(const std::string& name, const std::string& filepath, bool loop = true);
	bool LoadSE(const std::string& name, const std::string& filepath);
	
	// 재생
	void PlayBGM(const std::string& name);
	void StopBGM();
	void PlaySE(const std::string& name);
	void PlaySE_Force(const std::string& name);

	// 볼륨
	void SetMasterVolume(float volume); // [0.0 .. 1.0]
	void SetBGMVolume(float volume);    // [0.0 .. 1.0]
	void SetSEVolume(float volume);     // [0.0 .. 1.0]

	float GetMasterVolume() const { return m_MasterVolume; }
	float GetBGMVolume() const { return m_BGMVolume; }
	float GetSEVolume() const { return m_SEVolume; }

private:
	FmodSystemPtr m_System;

	// 이름 -> 사운드
	std::unordered_map<std::string, FmodSoundPtr> m_BGMs;
	std::unordered_map<std::string, FmodSoundPtr> m_SEs;

	std::unordered_map<std::string, FMOD::Channel*> m_SEChannels;

	// 현재 재생 중인 BGM 채널
	FMOD::Channel* m_BGMChannel{ nullptr };

	float m_MasterVolume{ 1.0f };
	float m_BGMVolume{ 1.0f };
	float m_SEVolume{ 1.0f };
};