#ifndef LINE_MAP_H
#define LINE_MAP_H


#include <vector>


using namespace std;


/* types */
typedef struct{
	unsigned int line;
	void *data;
} line_map_t;


class line_map{
public:
	line_map();
	~line_map();

	void add(unsigned int line, void *data);
	void *find(unsigned int line);
	void clear();

	vector<line_map_t> *lines();

private:
	vector<line_map_t> v;
};


#endif // LINE_MAP_H
