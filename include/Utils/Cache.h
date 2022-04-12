#pragma once

//#include <robin_hood.h>

namespace Utils
{
	// Inherit in custom derived CacheType<New derived class> class
	template<typename T> struct CacheType
	{
		virtual bool operator==(const T& x) const = 0;

		virtual size_t hash() const = 0;
	};

	// CacheTypeT = custom derived CacheType<New derived class> class, type T = m_LayoutCache[i].second
	//template<typename CacheTypeT, typename T> class Cache
	//{
	//	public:
	//		Cache() {};
	//		~Cache() {};
	//
	//		// Destroy all m_LayoutCache[i].second elements
	//		virtual void destroy() = 0;
	//	
	//	private:
	//		struct Hash
	//		{
	//			size_t operator()(const CacheTypeT& k)const {
	//				return k.hash();
	//			}
	//		};
	//
	//	protected:
	//		// Specific function for creating the specified type, cast void* to the desired type
	//		virtual T create(void* p_Info) = 0;
	//
	//		robin_hood::unordered_map<CacheTypeT, T, Hash> m_LayoutCache;
	//};
}