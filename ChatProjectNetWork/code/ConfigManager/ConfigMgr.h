#pragma once
#include "../const.h"
void TestRedis();
struct SectionInfo {
public:
	SectionInfo(){}
	~SectionInfo() { m_section_data.clear(); }
	SectionInfo(const SectionInfo& section) {
		m_section_data = section.m_section_data;
	}
	SectionInfo& operator=(const SectionInfo& section) {
		if (&section == this) {
			return *this;
		}
		m_section_data = section.m_section_data;
	}
	std::string operator[](const std::string& key) {
		if (m_section_data.find(key) == m_section_data.end()) {
			return "";
		}
		return m_section_data[key];
	}
public:
	std::map<std::string, std::string> m_section_data;

};
class ConfigMgr
{
public:
	ConfigMgr(const ConfigMgr& config);
	ConfigMgr& operator=(const ConfigMgr& config) {
		if (&config == this) {
			return *this;
		}
		this->m_config_map = config.m_config_map;
		return *this;
	}
	~ConfigMgr();
	SectionInfo operator[](const std::string& section) {
		if (m_config_map.find(section) == m_config_map.end()) {
			return SectionInfo();
		}
		return m_config_map[section];
	}
	static ConfigMgr& Inst() 
	{
		static ConfigMgr cfg_mgr;
		return cfg_mgr;
	}
private:
	ConfigMgr();
private:
	std::map<std::string, SectionInfo> m_config_map;
};

