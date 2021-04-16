#include <iostream>
#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <thread>

#include "httplib.h"
#include "utils.h"

using namespace httplib;

std::string dump_headers(const Headers& headers);
std::string log(const Request& req, const Response& res);

int main()
{
	std::thread t(workTobii);

	Server svr;
	if (!svr.is_valid()) 
	{
		std::cout << "server has an error..." << std::endl;
		return -1;
	}

	svr.Get("/start", [](const Request& /*req*/, Response& res) {
		if (status.length() < 2)
			status += status_start;
		else
			status = status_start;

		vec_time_stamp.clear();
		vec_data.clear();
		flag = true;
		});

	svr.Get("/end", [](const Request& /*req*/, Response& res) {
		if (status.length() < 2)
			status += status_end;
		else if (status[1] == '0')
			status = status_list[3];
		else
			status = status_list[5];

		if (status == status_list[3])
			res.set_content(dumpData(), "text/plain");
		flag = false;
		});

	svr.Get("/stop", [&](const Request& /*req*/, Response& /*res*/) { 
			flag = false;
			run_status = false;
			svr.stop(); 
		});

	svr.set_logger([](const Request& req, const Response& res) {
		printf("%s", log(req, res).c_str());
		});

	svr.listen("0.0.0.0", 20001);
	t.join();

	return 0;
}

std::string dump_headers(const Headers& headers)
{
	std::string s;
	char buf[BUFSIZ];

	for (auto it = headers.begin(); it != headers.end(); ++it)
	{
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

std::string log(const Request& req, const Response& res)
{
	std::string s;
	char buf[BUFSIZ];

	s += "================================\n";

	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	std::string query;
	for (auto it = req.params.begin(); it != req.params.end(); ++it)
	{
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%c%s=%s",
			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
			x.second.c_str());
		query += buf;
	}
	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
	s += buf;

	s += dump_headers(req.headers);
	s += "--------------------------------\n";

	/*snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
	s += buf;
	s += dump_headers(res.headers);
	s += "\n";*/

	// if (!res.body.empty()) { s += res.body; }

	// s += "\n";

	return s;
}