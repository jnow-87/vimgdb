#ifndef COMMON_MAP_H
#define COMMON_MAP_H


#define MAP_LOOKUP(map, val) ({ \
	auto it = (map).find(val); \
	it == (map).end() ? 0 : it->second; \
})

#define MAP_LOOKUP_SAFE(map, val, mtx) ({ \
	pthread_mutex_lock(&(mtx)); \
	auto it = (map).find(val); \
	pthread_mutex_unlock(&(mtx)); \
	\
	it == (map).end() ? 0 : it->second; \
})


#endif // COMMON_MAP_H
