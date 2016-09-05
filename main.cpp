#include <iostream>
#include <rest_rpc/server.hpp>
#include "registry.hpp"
#include "load_blancer.hpp"

struct configure
{
	int port;
	int thread_num;
	bool nodelay;

	META(port, thread_num, nodelay);
};

configure get_config()
{
	std::ifstream in("server.cfg");
	std::stringstream ss;
	ss << in.rdbuf();

	configure cfg = {};

	DeSerializer dr;
	try
	{
		dr.Parse(ss.str());
		dr.Deserialize(cfg);
	}
	catch (const std::exception& e)
	{
		timax::SPD_LOG_ERROR(e.what());
	}

	return cfg;
}

int main()
{
	using namespace timax::rpc;
	using timax::rpc::server;
	using codec_type = msgpack_codec;

	timax::log::get().init("rest_rpc_server.lg");
	auto cfg = get_config();
	int port = 9000;
	int thread_num = std::thread::hardware_concurrency();
	if (cfg.port != 0)
	{
		port = cfg.port;
		thread_num = cfg.thread_num;
	}

	auto sp = std::make_shared<server<codec_type>>(port, thread_num);

	registry reg;
	sp->register_handler("regist_service", &registry::regist_service, &reg);
	sp->register_handler("un_regist_service", &registry::un_regist_service, &reg);

	load_blancer bl;
	sp->register_handler("fetch", &load_blancer::fetch, &bl);

	sp->run();

	getchar();
	return 0;
}