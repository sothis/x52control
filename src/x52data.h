#ifndef X52DATA_H
#define X52DATA_H

#include <pthread.h>
#include <set>
#include "x52tools.h"

class x52data_t : public x52tools_t
{
	public:
		x52data_t(void);
		~x52data_t(void);
		
		void refresh_listeners(void);
		void add_listener(x52listener_t* listener);
		void remove_listener(x52listener_t* listener);
		void engage(float interval);
		void stop();
		const float& upd_interval(void);
		out_param_t& current_state(void);

		static float update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg);
	private:
		std::set<x52listener_t*> a_listeners;
		float a_interval;
		out_param_t current_data;
};

#endif /* X52DATA_H */

