#include <common/linemap.h>


line_map::line_map(){
}

line_map::~line_map(){
	clear();
}

void line_map::add(unsigned int line, void *data){
	v.push_back({ .line = line, .data = data});
}

void *line_map::find(unsigned int line){
	vector<line_map_t>::reverse_iterator it;


	for(it=v.rbegin(); it!=v.rend(); it++){
		if(it->line <= line)
			break;
	}

	if(it != v.rend())
		return it->data;
	return 0x0;
}

void line_map::clear(){
	v.clear();
}

vector<line_map_t> *line_map::lines(){
	return &v;
}
