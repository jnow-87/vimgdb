#ifndef COMMON_MAP_H
#define COMMON_MAP_H


#define MAP_LOOKUP(map, val)({ \
	auto it = (map).find(val); \
	it == (map).end() ? 0 : it->second; \
})

#define MAP_LOOKUP_SAFE(map, val, mtx)({ \
	pthread_mutex_lock(&(mtx)); \
	auto it = (map).find(val); \
	pthread_mutex_unlock(&(mtx)); \
	\
	it == (map).end() ? 0 : it->second; \
})

#define MAP_ERASE(map, val){ \
	auto it = (map).find(val); \
	if(it != (map).end()) \
		(map).erase(it); \
}

#define MAP_ERASE_SAFE(map, val, mtx){ \
	pthread_mutex_lock(&mtx); \
	auto it = (map).find(val); \
	if(it != (map).end()) \
		(map).erase(it); \
	pthread_mutex_unlock(&mtx); \
}


#endif // COMMON_MAP_H
