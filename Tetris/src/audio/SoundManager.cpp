#include "SoundManager.h"
#include "../utils/Logger.h"
#include <cassert>

SoundManager::~SoundManager()
{
	Shutdown();
}

bool SoundManager::Init()
{
	if (m_System) // 이미 초기화 되어있는 경우
		return true;

	FMOD::System* rawSystem = nullptr;
	FMOD_RESULT result = FMOD::System_Create(&rawSystem);
	if (result != FMOD_OK || !rawSystem)
		return false;

	// 소유권 이전
	m_System.reset(rawSystem);

	result = m_System->init(512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		// init 실패 시 시스템 해제
		m_System.reset();
		return false;
	}

	m_MasterVolume = 1.0f;
	m_BGMVolume = 1.0f;
	m_SEVolume = 1.0f;
	m_BGMChannel = nullptr;

	return true;
}

void SoundManager::Shutdown()
{
	if (m_BGMChannel)
	{
		m_BGMChannel->stop();
		m_BGMChannel = nullptr;
	}

	// 맵에 들어있는 사운드들은 unique_ptr로 알아서 release() 호출됨
	m_BGMs.clear();
	m_SEs.clear();
	m_SEChannels.clear();

	// 커스텀 deleter 호출
	if (m_System)
		m_System.reset();
}

void SoundManager::Update()
{
	if (m_System)
		m_System->update();
}

bool SoundManager::LoadBGM(const std::string& name, const std::string& filepath, bool loop)
{
	if (!m_System)
		return false;

	FMOD_MODE mode = FMOD_DEFAULT | FMOD_CREATESTREAM;
	mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

	FMOD::Sound* rawSound = nullptr;
	FMOD_RESULT result = m_System->createSound(filepath.c_str(), mode, nullptr, &rawSound);
	if (result != FMOD_OK || !rawSound)
		return false;

	m_BGMs[name] = FmodSoundPtr(rawSound);
	return true;
}

bool SoundManager::LoadSE(const std::string& name, const std::string& filepath)
{
	if (!m_System)
		return false;

	FMOD_MODE mode = FMOD_DEFAULT | FMOD_LOOP_OFF;

	FMOD::Sound* rawSound = nullptr;
	FMOD_RESULT result = m_System->createSound(filepath.c_str(), mode, nullptr, &rawSound);
	if (result != FMOD_OK || !rawSound)
		return false;

	m_SEs[name] = FmodSoundPtr(rawSound);
	return true;
}

void SoundManager::PlayBGM(const std::string& name)
{
	if (!m_System)
		return;

	auto it = m_BGMs.find(name);
	if (it == m_BGMs.end() || !it->second)  // sound가 없거나 null인 경우
		return;

	// 기존 BGM 정지
	if (m_BGMChannel) 
	{
		bool playing = false;
		m_BGMChannel->isPlaying(&playing);
		if (playing)
			m_BGMChannel->stop();
		m_BGMChannel = nullptr;
	}

	FMOD_RESULT result = m_System->playSound(it->second.get(), nullptr, false, &m_BGMChannel);
	if (result != FMOD_OK)
		return;

	if (m_BGMChannel) 
	{
		const float vol = m_MasterVolume * m_BGMVolume;
		m_BGMChannel->setVolume(vol);
	}
}

void SoundManager::StopBGM()
{
	if (m_BGMChannel) 
	{
		m_BGMChannel->stop();
		m_BGMChannel = nullptr;
	}
}

void SoundManager::PlaySE(const std::string& name)
{
	if (!m_System)
		return;

	auto it = m_SEs.find(name);
	if (it == m_SEs.end() || !it->second)
		return;

	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = m_System->playSound(it->second.get(), nullptr, false, &channel);
	if (result != FMOD_OK)
	{
		assert(false && "PlaySE Failed");
		return;
	}

	if (channel) 
	{
		const float vol = m_MasterVolume * m_SEVolume;
		channel->setVolume(vol);
	}
}

void SoundManager::PlaySE_Force(const std::string& name)
{
	if (!m_System)
	{
		__debugbreak();
		return;
	}

	auto sound_it = m_SEs.find(name);
	if (sound_it == m_SEs.end() || !sound_it->second)
	{
		__debugbreak();
		return;
	}

	auto& channel = m_SEChannels[name];
	if (channel)
	{
		// 이미 재생중이라면 중단
		bool isPlaying = false;
		if (channel->isPlaying(&isPlaying) == FMOD_OK && isPlaying)
		{
			channel->stop();
		}
	}

	FMOD::Channel* newChannel = nullptr;
	FMOD_RESULT result = m_System->playSound(sound_it->second.get(), nullptr, false, &newChannel);
	if (result != FMOD_OK)
	{
		assert(false && "PlaySE_ForceRestart Failed");
		return;
	}

	const float vol = m_MasterVolume * m_SEVolume;
	newChannel->setVolume(vol);
	channel = newChannel;
}

void SoundManager::SetMasterVolume(float volume)
{
	m_MasterVolume = std::clamp(volume, 0.0f, 1.0f);

	// 현재 재생 중인 BGM에 즉시 반영
	if (m_BGMChannel) 
	{
		const float vol = m_MasterVolume * m_BGMVolume;
		m_BGMChannel->setVolume(vol);
	}
}

void SoundManager::SetBGMVolume(float volume)
{
	m_BGMVolume = std::clamp(volume, 0.0f, 1.0f);

	if (m_BGMChannel) 
	{
		const float vol = m_MasterVolume * m_BGMVolume;
		m_BGMChannel->setVolume(vol);
	}
}

void SoundManager::SetSEVolume(float volume)
{
	m_SEVolume = std::clamp(volume, 0.0f, 1.0f);
}
