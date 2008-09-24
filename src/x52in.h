#ifndef X52IN_H
#define X52IN_H

#include <set>
#include "x52tools.h"

class x52mfdpage_t;

class x52in_t : public x52tools_t
{
	public:
		x52in_t(void);
		~x52in_t(void);
		void handle_pagecycle(void);
		void add_page(x52mfdpage_t* page);
		static int dispatch_command(void* cmd, int phase, void *arg);
		void*	a_cmd_pagecycle;
	private:
		std::set<x52mfdpage_t*> a_pages;
		x52mfdpage_t* a_currentpage;
};

#endif /* X52IN_H */

