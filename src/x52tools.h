#ifndef X52TOOLS_H
#define X52TOOLS_H

typedef struct out_param_t
{
	float	next_call;
	char	text[51];
} out_param_t;

class x52tools_t
{
	public:
		x52tools_t(void);
		void debug_out(int type, const char* msg, ...);
		static void debug_out(const char* msg, ...);
		bool verbose;
		enum dbgtype_e
		{
			info,
			warn,
			err
		};
	private:
};

class x52listener_t
{
	public:
		virtual void refresh(void* param){};
};

#endif /* X52TOOLS_H */

