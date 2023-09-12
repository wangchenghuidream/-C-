#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include<memory>
#include<sstream>
#include<vector>
#include<list>
#include<map>

#include<boost/lexical_cast.hpp>
#include<yaml-cpp/yaml.h>
#include "log.h"

namespace sylar
{

//配置变量的基类
class ConfigVarBase
{
public:
	typedef std::shared_ptr<ConfigVarBase> ptr;
	
	//构造函数，初始化配置变量名称和描述
	ConfigVarBase(const std::string& name, const std::string& description = "")
	:m_name(name), m_description(description) 
	{
		std::transform(name.begin(), name.end(), ::tolower));
	}
	
	//析构函数
	virtual ~ConfigVarBase() {}
	
	//返回配置变量名称
	const std::string& getName() {return m_name;}
	
	//返回配置变量描述
	const std::string& getDescription() {return m_description;}
	
	//转为字符串
	virtual std::string toString() = 0;
	
	//从字符串初始值
	virtual bool fromString(const std::string& val) =0;

protected:
	std::string m_name;
	std::string m_description;
};

//通用类型转换
template<class F, class T>
class LexicalCast
{
public:
	T operator()(const F& v)
	{
		return boost::lexical_cast<T>(v);
	}
}

//vector偏特化
template<class T>
class LexicalCast<std::string, std::vector<T>>
{
public:
	std::vector<T> operator()(const std::string& v)
	{
		YAML::Node node = YAML::Load(v);
		typename std::vector<T> vec;
		std::stringstream ss;
		for(size_t i =0; i< node.size(); ++i)
		{
			ss.str("");
			ss<<node[i];
			vec.push_back(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
}

template<class T>
class LexicalCast<std::vector<T>, std::string>
{
public:
	std::string operator()(const std::vector<T>& v)
	{
		YAML::Node node;
		for(auto& i:v)
		{
			node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
		}
		std::stringstream ss;
		ss<<node;
		return ss.str();
	}
}

//list偏特化
template<class T>
class LexicalCast<std::string, std::list<T>>
{
public:
	std::list<T> operator()(const std::string& v)
	{
		YAML::Node node = YAML::Load(v);
		typename std::list<T> vec;
		std::stringstream ss;
		for(size_t i =0; i< node.size(); ++i)
		{
			ss.str("");
			ss<<node[i];
			vec.push_back(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
}

template<class T>
class LexicalCast<std::list<T>, std::string>
{
public:
	std::string operator()(const std::list<T>& v)
	{
		YAML::Node node;
		for(auto& i:v)
		{
			node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
		}
		std::stringstream ss;
		ss<<node;
		return ss.str();
	}
}

//set偏特化
template<class T>
class LexicalCast<std::string, std::set<T>>
{
public:
	std::set<T> operator()(const std::string& v)
	{
		YAML::Node node = YAML::Load(v);
		typename std::set<T> vec;
		std::stringstream ss;
		for(size_t i =0; i< node.size(); ++i)
		{
			ss.str("");
			ss<<node[i];
			vec.insert(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
}

template<class T>
class LexicalCast<std::set<T>, std::string>
{
public:
	std::string operator()(const std::set<T>& v)
	{
		YAML::Node node;
		for(auto& i:v)
		{
			node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
		}
		std::stringstream ss;
		ss<<node;
		return ss.str();
	}
}

//unordered_set偏特化
template<class T>
class LexicalCast<std::string, std::unordered_set<T>>
{
public:
	std::unordered_set<T> operator()(const std::string& v)
	{
		YAML::Node node = YAML::Load(v);
		typename std::unordered_set<T> vec;
		std::stringstream ss;
		for(size_t i =0; i< node.size(); ++i)
		{
			ss.str("");
			ss<<node[i];
			vec.insert(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
}

template<class T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
	std::string operator()(const std::unordered_set<T>& v)
	{
		YAML::Node node;
		for(auto& i:v)
		{
			node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
		}
		std::stringstream ss;
		ss<<node;
		return ss.str();
	}
}

//map偏特化
template<class T>
class LexicalCast<std::string, std::map<std::string,T>>
{
public:
	std::map<std::string, T> operator()(const std::string& v)
	{
		YAML::Node node = YAML::Load(v);
		typename std::map<std::string,T> vec;
		std::stringstream ss;
		for(auto it = node.begin(); it != node.end(); ++i)
		{
			ss.str("");
			ss<< it->second;
			vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string, T>()(ss.str())));
		}
		return vec;
	}
}

template<class T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
	std::string operator()(const std::map<std::string, T>& v)
	{
		YAML::Node node;
		for(auto& i:v)
		{
			node[i.first] = YAML::Load(LexicalCast<T,std::string>()(i.second));
		}
		std::stringstream ss;
		ss<<node;
		return ss.str();
	}
}

//map偏特化
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string,T>>
{
public:
	std::unordered_map<std::string, T> operator()(const std::string& v)
	{
		YAML::Node node = YAML::Load(v);
		typename std::unordered_map<std::string,T> vec;
		std::stringstream ss;
		for(auto it = node.begin(); it != node.end(); ++i)
		{
			ss.str("");
			ss<< it->second;
			vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string, T>()(ss.str())));
		}
		return vec;
	}
}

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
	std::string operator()(const std::unordered_map<std::string, T>& v)
	{
		YAML::Node node;
		for(auto& i:v)
		{
			node[i.first] = YAML::Load(LexicalCast<T,std::string>()(i.second));
		}
		std::stringstream ss;
		ss<<node;
		return ss.str();
	}
}

//以模板定义配置参数子类
//FromStr T operator()(const std::string& )
//ToStr std::string operator()(const T&)
template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase
{
public:
	typedef std::shared_ptr<ConfigVar> ptr;
	typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;
	ConfigVar(const std::string& name, const T& default_value, const std::string description="")
	:ConfigVarBase(name, description), m_val(default_value) {}
	
	std::string toString() override
	{
		try
		{
			//return boost::lexical_cast<std::string>(m_val);
			return ToStr()(m_val);
		}
		catch(std::exception& e){
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT) <<"ConfigVar::toString exception"
			<<e.what() << "convert:" <<typeid(m_val).name() <<"to string";
		}
		return "";
	}
	
	bool fromString(const std::string& val) override
	{
		try
		{
			//m_val = boost::lexical_cast<T>(val);
			setValue(FromStr()(val));
		}
		catch(std::exception& e)
		{
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT) <<"ConfigVar::fromString exception"
			<<e.what() << "convert:string to" <<typeid(m_val).name();
		}
		return false
	}
	
	const T getValue() const {return m_val;}
	
	void setValue(const T& v) 
	{
		if(m_val == v)
			return;
		for(auto& i : m_cbs)
		{
			i.second(m_val, v);
		}
		m_val = v;
	}
	
	std::string getTypeName() const override {return TypeToName<T>();}
	
	
	//添加变化回调函数
	void addListener(uint64_t key, on_change_cb cb)
	{
		m_cbs[key] = cb;
	}
	
	//删除变化回调函数
	void delListener(uint64_t key)
	{
		m_cbs.erase(key);
	}
	
	//获取回调函数
	on_change_cb getListener(uint64_t key)
	{
		auto it = m_cbs.find(key);
		return it == m_cbs.end()?nullptr:it->second;
	}
	
	//清理所有的回调函数
	void clearListener()
	{
		m_cbs.clear();
	}
	
private:
	T m_val;
	//变更回调函数数组，uint64_t key要求唯一，一般可以用hash
	//为什么用map,function对象没有比较函数，必须通过另一种方式进行查找
	std::map<uint64_t, on_change_cb> m_cbs;
};
	
//配置变量的管理类
//提供便捷的方法创建/管理配置变量
class Config
{
public:
	typedef std::unordered_map<std:string, ConfigVarBase::ptr> ConfigVarMap;
	
	template<class T>
	static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value, const std::string& description="")
	{
		auto it = s_datas.find(name);
		if(it != s_datas.end())
		{
			auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
			if(tmp)
			{
				SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name="<<name<<"exists";
				return tmp;
			}
			else
			{
				SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name="<<name<<"exists but type not" 
				<< typeid(T).name()<<"real_type="<<it->second->getTypeName()
				<<" "<<it->second->toString();
				return nullptr;
			}
		}
		if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
		{
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid" <<name;
			throw std::invalid_argument(name);
		}
		typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
		s_datas[name] = v;

	}
	
	template<class T>
	static typename ConfigVar<T>::ptr Lookup(const std::string& name)
	{
		auto it = s_datas.find(name);
		if(it == s_datas.end())
		{
			return nullptr;
		}
		return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
	}
	
	//使用YAML::Node初始化配置模块
	static void LoadFromYaml(const YAML::Node& root);
	
	//按名称查找配置参数，返回其基类
	static ConfigVarBase::ptr LookupBase(const std::string& name);

private:
	static ConfigVarMap s_datas;

}

	
}

#endif // CONFIG_H_INCLUDED